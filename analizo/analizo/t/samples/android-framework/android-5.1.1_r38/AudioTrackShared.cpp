/*

**ANALIZO NOTE**

This file was copied from The Android Open Source Project for testing analizo
features.

The original file was copied as-is to create automated tests on analizo side
fixing the bug below.

- https://github.com/analizo/analizo/issues/112

Android source code repositories:

- https://android.googlesource.com/

Original file was copied from the tag android-5.1.1_r38 from Android git repository
and it is located on the path below.

- media/libmedia/AudioTrackShared.cpp

Link to the original file on git repository:

- https://android.googlesource.com/platform/frameworks/av/+/refs/tags/android-5.1.1_r38/media/libmedia/AudioTrackShared.cpp

*/

/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "AudioTrackShared"
//#define LOG_NDEBUG 0

#include <private/media/AudioTrackShared.h>
#include <utils/Log.h>

#include <linux/futex.h>
#include <sys/syscall.h>

namespace android {

// used to clamp a value to size_t.  TODO: move to another file.
template <typename T>
size_t clampToSize(T x) {
    return x > SIZE_MAX ? SIZE_MAX : x < 0 ? 0 : (size_t) x;
}

audio_track_cblk_t::audio_track_cblk_t()
    : mServer(0), mFutex(0), mMinimum(0),
    mVolumeLR(GAIN_MINIFLOAT_PACKED_UNITY), mSampleRate(0), mSendLevel(0), mFlags(0)
{
    memset(&u, 0, sizeof(u));
}

// ---------------------------------------------------------------------------

Proxy::Proxy(audio_track_cblk_t* cblk, void *buffers, size_t frameCount, size_t frameSize,
        bool isOut, bool clientInServer)
    : mCblk(cblk), mBuffers(buffers), mFrameCount(frameCount), mFrameSize(frameSize),
      mFrameCountP2(roundup(frameCount)), mIsOut(isOut), mClientInServer(clientInServer),
      mIsShutdown(false), mUnreleased(0)
{
}

// ---------------------------------------------------------------------------

ClientProxy::ClientProxy(audio_track_cblk_t* cblk, void *buffers, size_t frameCount,
        size_t frameSize, bool isOut, bool clientInServer)
    : Proxy(cblk, buffers, frameCount, frameSize, isOut, clientInServer), mEpoch(0)
{
}

const struct timespec ClientProxy::kForever = {INT_MAX /*tv_sec*/, 0 /*tv_nsec*/};
const struct timespec ClientProxy::kNonBlocking = {0 /*tv_sec*/, 0 /*tv_nsec*/};

#define MEASURE_NS 10000000 // attempt to provide accurate timeouts if requested >= MEASURE_NS

// To facilitate quicker recovery from server failure, this value limits the timeout per each futex
// wait.  However it does not protect infinite timeouts.  If defined to be zero, there is no limit.
// FIXME May not be compatible with audio tunneling requirements where timeout should be in the
// order of minutes.
#define MAX_SEC    5

status_t ClientProxy::obtainBuffer(Buffer* buffer, const struct timespec *requested,
        struct timespec *elapsed)
{
    LOG_ALWAYS_FATAL_IF(buffer == NULL || buffer->mFrameCount == 0);
    struct timespec total;          // total elapsed time spent waiting
    total.tv_sec = 0;
    total.tv_nsec = 0;
    bool measure = elapsed != NULL; // whether to measure total elapsed time spent waiting

    status_t status;
    enum {
        TIMEOUT_ZERO,       // requested == NULL || *requested == 0
        TIMEOUT_INFINITE,   // *requested == infinity
        TIMEOUT_FINITE,     // 0 < *requested < infinity
        TIMEOUT_CONTINUE,   // additional chances after TIMEOUT_FINITE
    } timeout;
    if (requested == NULL) {
        timeout = TIMEOUT_ZERO;
    } else if (requested->tv_sec == 0 && requested->tv_nsec == 0) {
        timeout = TIMEOUT_ZERO;
    } else if (requested->tv_sec == INT_MAX) {
        timeout = TIMEOUT_INFINITE;
    } else {
        timeout = TIMEOUT_FINITE;
        if (requested->tv_sec > 0 || requested->tv_nsec >= MEASURE_NS) {
            measure = true;
        }
    }
    struct timespec before;
    bool beforeIsValid = false;
    audio_track_cblk_t* cblk = mCblk;
    bool ignoreInitialPendingInterrupt = true;
    // check for shared memory corruption
    if (mIsShutdown) {
        status = NO_INIT;
        goto end;
    }
    for (;;) {
        int32_t flags = android_atomic_and(~CBLK_INTERRUPT, &cblk->mFlags);
        // check for track invalidation by server, or server death detection
        if (flags & CBLK_INVALID) {
            ALOGV("Track invalidated");
            status = DEAD_OBJECT;
            goto end;
        }
        // check for obtainBuffer interrupted by client
        if (!ignoreInitialPendingInterrupt && (flags & CBLK_INTERRUPT)) {
            ALOGV("obtainBuffer() interrupted by client");
            status = -EINTR;
            goto end;
        }
        ignoreInitialPendingInterrupt = false;
        // compute number of frames available to write (AudioTrack) or read (AudioRecord)
        int32_t front;
        int32_t rear;
        if (mIsOut) {
            // The barrier following the read of mFront is probably redundant.
            // We're about to perform a conditional branch based on 'filled',
            // which will force the processor to observe the read of mFront
            // prior to allowing data writes starting at mRaw.
            // However, the processor may support speculative execution,
            // and be unable to undo speculative writes into shared memory.
            // The barrier will prevent such speculative execution.
            front = android_atomic_acquire_load(&cblk->u.mStreaming.mFront);
            rear = cblk->u.mStreaming.mRear;
        } else {
            // On the other hand, this barrier is required.
            rear = android_atomic_acquire_load(&cblk->u.mStreaming.mRear);
            front = cblk->u.mStreaming.mFront;
        }
        ssize_t filled = rear - front;
        // pipe should not be overfull
        if (!(0 <= filled && (size_t) filled <= mFrameCount)) {
            if (mIsOut) {
                ALOGE("Shared memory control block is corrupt (filled=%zd, mFrameCount=%zu); "
                        "shutting down", filled, mFrameCount);
                mIsShutdown = true;
                status = NO_INIT;
                goto end;
            }
            // for input, sync up on overrun
            filled = 0;
            cblk->u.mStreaming.mFront = rear;
            (void) android_atomic_or(CBLK_OVERRUN, &cblk->mFlags);
        }
        // don't allow filling pipe beyond the nominal size
        size_t avail = mIsOut ? mFrameCount - filled : filled;
        if (avail > 0) {
            // 'avail' may be non-contiguous, so return only the first contiguous chunk
            size_t part1;
            if (mIsOut) {
                rear &= mFrameCountP2 - 1;
                part1 = mFrameCountP2 - rear;
            } else {
                front &= mFrameCountP2 - 1;
                part1 = mFrameCountP2 - front;
            }
            if (part1 > avail) {
                part1 = avail;
            }
            if (part1 > buffer->mFrameCount) {
                part1 = buffer->mFrameCount;
            }
            buffer->mFrameCount = part1;
            buffer->mRaw = part1 > 0 ?
                    &((char *) mBuffers)[(mIsOut ? rear : front) * mFrameSize] : NULL;
            buffer->mNonContig = avail - part1;
            mUnreleased = part1;
            status = NO_ERROR;
            break;
        }
        struct timespec remaining;
        const struct timespec *ts;
        switch (timeout) {
        case TIMEOUT_ZERO:
            status = WOULD_BLOCK;
            goto end;
        case TIMEOUT_INFINITE:
            ts = NULL;
            break;
        case TIMEOUT_FINITE:
            timeout = TIMEOUT_CONTINUE;
            if (MAX_SEC == 0) {
                ts = requested;
                break;
            }
            // fall through
        case TIMEOUT_CONTINUE:
            // FIXME we do not retry if requested < 10ms? needs documentation on this state machine
            if (!measure || requested->tv_sec < total.tv_sec ||
                    (requested->tv_sec == total.tv_sec && requested->tv_nsec <= total.tv_nsec)) {
                status = TIMED_OUT;
                goto end;
            }
            remaining.tv_sec = requested->tv_sec - total.tv_sec;
            if ((remaining.tv_nsec = requested->tv_nsec - total.tv_nsec) < 0) {
                remaining.tv_nsec += 1000000000;
                remaining.tv_sec++;
            }
            if (0 < MAX_SEC && MAX_SEC < remaining.tv_sec) {
                remaining.tv_sec = MAX_SEC;
                remaining.tv_nsec = 0;
            }
            ts = &remaining;
            break;
        default:
            LOG_ALWAYS_FATAL("obtainBuffer() timeout=%d", timeout);
            ts = NULL;
            break;
        }
        int32_t old = android_atomic_and(~CBLK_FUTEX_WAKE, &cblk->mFutex);
        if (!(old & CBLK_FUTEX_WAKE)) {
            if (measure && !beforeIsValid) {
                clock_gettime(CLOCK_MONOTONIC, &before);
                beforeIsValid = true;
            }
            errno = 0;
            (void) syscall(__NR_futex, &cblk->mFutex,
                    mClientInServer ? FUTEX_WAIT_PRIVATE : FUTEX_WAIT, old & ~CBLK_FUTEX_WAKE, ts);
            // update total elapsed time spent waiting
            if (measure) {
                struct timespec after;
                clock_gettime(CLOCK_MONOTONIC, &after);
                total.tv_sec += after.tv_sec - before.tv_sec;
                long deltaNs = after.tv_nsec - before.tv_nsec;
                if (deltaNs < 0) {
                    deltaNs += 1000000000;
                    total.tv_sec--;
                }
                if ((total.tv_nsec += deltaNs) >= 1000000000) {
                    total.tv_nsec -= 1000000000;
                    total.tv_sec++;
                }
                before = after;
                beforeIsValid = true;
            }
            switch (errno) {
            case 0:            // normal wakeup by server, or by binderDied()
            case EWOULDBLOCK:  // benign race condition with server
            case EINTR:        // wait was interrupted by signal or other spurious wakeup
            case ETIMEDOUT:    // time-out expired
                // FIXME these error/non-0 status are being dropped
                break;
            default:
                status = errno;
                ALOGE("%s unexpected error %s", __func__, strerror(status));
                goto end;
            }
        }
    }

end:
    if (status != NO_ERROR) {
        buffer->mFrameCount = 0;
        buffer->mRaw = NULL;
        buffer->mNonContig = 0;
        mUnreleased = 0;
    }
    if (elapsed != NULL) {
        *elapsed = total;
    }
    if (requested == NULL) {
        requested = &kNonBlocking;
    }
    if (measure) {
        ALOGV("requested %ld.%03ld elapsed %ld.%03ld",
              requested->tv_sec, requested->tv_nsec / 1000000,
              total.tv_sec, total.tv_nsec / 1000000);
    }
    return status;
}

void ClientProxy::releaseBuffer(Buffer* buffer)
{
    LOG_ALWAYS_FATAL_IF(buffer == NULL);
    size_t stepCount = buffer->mFrameCount;
    if (stepCount == 0 || mIsShutdown) {
        // prevent accidental re-use of buffer
        buffer->mFrameCount = 0;
        buffer->mRaw = NULL;
        buffer->mNonContig = 0;
        return;
    }
    LOG_ALWAYS_FATAL_IF(!(stepCount <= mUnreleased && mUnreleased <= mFrameCount));
    mUnreleased -= stepCount;
    audio_track_cblk_t* cblk = mCblk;
    // Both of these barriers are required
    if (mIsOut) {
        int32_t rear = cblk->u.mStreaming.mRear;
        android_atomic_release_store(stepCount + rear, &cblk->u.mStreaming.mRear);
    } else {
        int32_t front = cblk->u.mStreaming.mFront;
        android_atomic_release_store(stepCount + front, &cblk->u.mStreaming.mFront);
    }
}

void ClientProxy::binderDied()
{
    audio_track_cblk_t* cblk = mCblk;
    if (!(android_atomic_or(CBLK_INVALID, &cblk->mFlags) & CBLK_INVALID)) {
        android_atomic_or(CBLK_FUTEX_WAKE, &cblk->mFutex);
        // it seems that a FUTEX_WAKE_PRIVATE will not wake a FUTEX_WAIT, even within same process
        (void) syscall(__NR_futex, &cblk->mFutex, mClientInServer ? FUTEX_WAKE_PRIVATE : FUTEX_WAKE,
                1);
    }
}

void ClientProxy::interrupt()
{
    audio_track_cblk_t* cblk = mCblk;
    if (!(android_atomic_or(CBLK_INTERRUPT, &cblk->mFlags) & CBLK_INTERRUPT)) {
        android_atomic_or(CBLK_FUTEX_WAKE, &cblk->mFutex);
        (void) syscall(__NR_futex, &cblk->mFutex, mClientInServer ? FUTEX_WAKE_PRIVATE : FUTEX_WAKE,
                1);
    }
}

size_t ClientProxy::getMisalignment()
{
    audio_track_cblk_t* cblk = mCblk;
    return (mFrameCountP2 - (mIsOut ? cblk->u.mStreaming.mRear : cblk->u.mStreaming.mFront)) &
            (mFrameCountP2 - 1);
}

size_t ClientProxy::getFramesFilled() {
    audio_track_cblk_t* cblk = mCblk;
    int32_t front;
    int32_t rear;

    if (mIsOut) {
        front = android_atomic_acquire_load(&cblk->u.mStreaming.mFront);
        rear = cblk->u.mStreaming.mRear;
    } else {
        rear = android_atomic_acquire_load(&cblk->u.mStreaming.mRear);
        front = cblk->u.mStreaming.mFront;
    }
    ssize_t filled = rear - front;
    // pipe should not be overfull
    if (!(0 <= filled && (size_t) filled <= mFrameCount)) {
        ALOGE("Shared memory control block is corrupt (filled=%zd); shutting down", filled);
        return 0;
    }
    return (size_t)filled;
}

// ---------------------------------------------------------------------------

void AudioTrackClientProxy::flush()
{
    // This works for mFrameCountP2 <= 2^30
    size_t increment = mFrameCountP2 << 1;
    size_t mask = increment - 1;
    audio_track_cblk_t* cblk = mCblk;
    int32_t newFlush = (cblk->u.mStreaming.mRear & mask) |
                        ((cblk->u.mStreaming.mFlush & ~mask) + increment);
    android_atomic_release_store(newFlush, &cblk->u.mStreaming.mFlush);
}

bool AudioTrackClientProxy::clearStreamEndDone() {
    return (android_atomic_and(~CBLK_STREAM_END_DONE, &mCblk->mFlags) & CBLK_STREAM_END_DONE) != 0;
}

bool AudioTrackClientProxy::getStreamEndDone() const {
    return (mCblk->mFlags & CBLK_STREAM_END_DONE) != 0;
}

status_t AudioTrackClientProxy::waitStreamEndDone(const struct timespec *requested)
{
    struct timespec total;          // total elapsed time spent waiting
    total.tv_sec = 0;
    total.tv_nsec = 0;
    audio_track_cblk_t* cblk = mCblk;
    status_t status;
    enum {
        TIMEOUT_ZERO,       // requested == NULL || *requested == 0
        TIMEOUT_INFINITE,   // *requested == infinity
        TIMEOUT_FINITE,     // 0 < *requested < infinity
        TIMEOUT_CONTINUE,   // additional chances after TIMEOUT_FINITE
    } timeout;
    if (requested == NULL) {
        timeout = TIMEOUT_ZERO;
    } else if (requested->tv_sec == 0 && requested->tv_nsec == 0) {
        timeout = TIMEOUT_ZERO;
    } else if (requested->tv_sec == INT_MAX) {
        timeout = TIMEOUT_INFINITE;
    } else {
        timeout = TIMEOUT_FINITE;
    }
    for (;;) {
        int32_t flags = android_atomic_and(~(CBLK_INTERRUPT|CBLK_STREAM_END_DONE), &cblk->mFlags);
        // check for track invalidation by server, or server death detection
        if (flags & CBLK_INVALID) {
            ALOGV("Track invalidated");
            status = DEAD_OBJECT;
            goto end;
        }
        if (flags & CBLK_STREAM_END_DONE) {
            ALOGV("stream end received");
            status = NO_ERROR;
            goto end;
        }
        // check for obtainBuffer interrupted by client
        // check for obtainBuffer interrupted by client
        if (flags & CBLK_INTERRUPT) {
            ALOGV("waitStreamEndDone() interrupted by client");
            status = -EINTR;
            goto end;
        }
        struct timespec remaining;
        const struct timespec *ts;
        switch (timeout) {
        case TIMEOUT_ZERO:
            status = WOULD_BLOCK;
            goto end;
        case TIMEOUT_INFINITE:
            ts = NULL;
            break;
        case TIMEOUT_FINITE:
            timeout = TIMEOUT_CONTINUE;
            if (MAX_SEC == 0) {
                ts = requested;
                break;
            }
            // fall through
        case TIMEOUT_CONTINUE:
            // FIXME we do not retry if requested < 10ms? needs documentation on this state machine
            if (requested->tv_sec < total.tv_sec ||
                    (requested->tv_sec == total.tv_sec && requested->tv_nsec <= total.tv_nsec)) {
                status = TIMED_OUT;
                goto end;
            }
            remaining.tv_sec = requested->tv_sec - total.tv_sec;
            if ((remaining.tv_nsec = requested->tv_nsec - total.tv_nsec) < 0) {
                remaining.tv_nsec += 1000000000;
                remaining.tv_sec++;
            }
            if (0 < MAX_SEC && MAX_SEC < remaining.tv_sec) {
                remaining.tv_sec = MAX_SEC;
                remaining.tv_nsec = 0;
            }
            ts = &remaining;
            break;
        default:
            LOG_ALWAYS_FATAL("waitStreamEndDone() timeout=%d", timeout);
            ts = NULL;
            break;
        }
        int32_t old = android_atomic_and(~CBLK_FUTEX_WAKE, &cblk->mFutex);
        if (!(old & CBLK_FUTEX_WAKE)) {
            errno = 0;
            (void) syscall(__NR_futex, &cblk->mFutex,
                    mClientInServer ? FUTEX_WAIT_PRIVATE : FUTEX_WAIT, old & ~CBLK_FUTEX_WAKE, ts);
            switch (errno) {
            case 0:            // normal wakeup by server, or by binderDied()
            case EWOULDBLOCK:  // benign race condition with server
            case EINTR:        // wait was interrupted by signal or other spurious wakeup
            case ETIMEDOUT:    // time-out expired
                break;
            default:
                status = errno;
                ALOGE("%s unexpected error %s", __func__, strerror(status));
                goto end;
            }
        }
    }

end:
    if (requested == NULL) {
        requested = &kNonBlocking;
    }
    return status;
}

// ---------------------------------------------------------------------------

StaticAudioTrackClientProxy::StaticAudioTrackClientProxy(audio_track_cblk_t* cblk, void *buffers,
        size_t frameCount, size_t frameSize)
    : AudioTrackClientProxy(cblk, buffers, frameCount, frameSize),
      mMutator(&cblk->u.mStatic.mSingleStateQueue), mBufferPosition(0)
{
}

void StaticAudioTrackClientProxy::flush()
{
    LOG_ALWAYS_FATAL("static flush");
}

void StaticAudioTrackClientProxy::setLoop(size_t loopStart, size_t loopEnd, int loopCount)
{
    // This can only happen on a 64-bit client
    if (loopStart > UINT32_MAX || loopEnd > UINT32_MAX) {
        // FIXME Should return an error status
        return;
    }
    StaticAudioTrackState newState;
    newState.mLoopStart = (uint32_t) loopStart;
    newState.mLoopEnd = (uint32_t) loopEnd;
    newState.mLoopCount = loopCount;
    size_t bufferPosition;
    if (loopCount == 0 || (bufferPosition = getBufferPosition()) >= loopEnd) {
        bufferPosition = loopStart;
    }
    mBufferPosition = bufferPosition; // snapshot buffer position until loop is acknowledged.
    (void) mMutator.push(newState);
}

size_t StaticAudioTrackClientProxy::getBufferPosition()
{
    size_t bufferPosition;
    if (mMutator.ack()) {
        bufferPosition = (size_t) mCblk->u.mStatic.mBufferPosition;
        if (bufferPosition > mFrameCount) {
            bufferPosition = mFrameCount;
        }
    } else {
        bufferPosition = mBufferPosition;
    }
    return bufferPosition;
}

// ---------------------------------------------------------------------------

ServerProxy::ServerProxy(audio_track_cblk_t* cblk, void *buffers, size_t frameCount,
        size_t frameSize, bool isOut, bool clientInServer)
    : Proxy(cblk, buffers, frameCount, frameSize, isOut, clientInServer),
      mAvailToClient(0), mFlush(0)
{
}

status_t ServerProxy::obtainBuffer(Buffer* buffer, bool ackFlush)
{
    LOG_ALWAYS_FATAL_IF(buffer == NULL || buffer->mFrameCount == 0);
    if (mIsShutdown) {
        goto no_init;
    }
    {
    audio_track_cblk_t* cblk = mCblk;
    // compute number of frames available to write (AudioTrack) or read (AudioRecord),
    // or use previous cached value from framesReady(), with added barrier if it omits.
    int32_t front;
    int32_t rear;
    // See notes on barriers at ClientProxy::obtainBuffer()
    if (mIsOut) {
        int32_t flush = cblk->u.mStreaming.mFlush;
        rear = android_atomic_acquire_load(&cblk->u.mStreaming.mRear);
        front = cblk->u.mStreaming.mFront;
        if (flush != mFlush) {
            // effectively obtain then release whatever is in the buffer
            size_t mask = (mFrameCountP2 << 1) - 1;
            int32_t newFront = (front & ~mask) | (flush & mask);
            ssize_t filled = rear - newFront;
            // Rather than shutting down on a corrupt flush, just treat it as a full flush
            if (!(0 <= filled && (size_t) filled <= mFrameCount)) {
                ALOGE("mFlush %#x -> %#x, front %#x, rear %#x, mask %#x, newFront %#x, filled %d=%#x",
                        mFlush, flush, front, rear, mask, newFront, filled, filled);
                newFront = rear;
            }
            mFlush = flush;
            android_atomic_release_store(newFront, &cblk->u.mStreaming.mFront);
            // There is no danger from a false positive, so err on the side of caution
            if (true /*front != newFront*/) {
                int32_t old = android_atomic_or(CBLK_FUTEX_WAKE, &cblk->mFutex);
                if (!(old & CBLK_FUTEX_WAKE)) {
                    (void) syscall(__NR_futex, &cblk->mFutex,
                            mClientInServer ? FUTEX_WAKE_PRIVATE : FUTEX_WAKE, 1);
                }
            }
            front = newFront;
        }
    } else {
        front = android_atomic_acquire_load(&cblk->u.mStreaming.mFront);
        rear = cblk->u.mStreaming.mRear;
    }
    ssize_t filled = rear - front;
    // pipe should not already be overfull
    if (!(0 <= filled && (size_t) filled <= mFrameCount)) {
        ALOGE("Shared memory control block is corrupt (filled=%zd); shutting down", filled);
        mIsShutdown = true;
    }
    if (mIsShutdown) {
        goto no_init;
    }
    // don't allow filling pipe beyond the nominal size
    size_t availToServer;
    if (mIsOut) {
        availToServer = filled;
        mAvailToClient = mFrameCount - filled;
    } else {
        availToServer = mFrameCount - filled;
        mAvailToClient = filled;
    }
    // 'availToServer' may be non-contiguous, so return only the first contiguous chunk
    size_t part1;
    if (mIsOut) {
        front &= mFrameCountP2 - 1;
        part1 = mFrameCountP2 - front;
    } else {
        rear &= mFrameCountP2 - 1;
        part1 = mFrameCountP2 - rear;
    }
    if (part1 > availToServer) {
        part1 = availToServer;
    }
    size_t ask = buffer->mFrameCount;
    if (part1 > ask) {
        part1 = ask;
    }
    // is assignment redundant in some cases?
    buffer->mFrameCount = part1;
    buffer->mRaw = part1 > 0 ?
            &((char *) mBuffers)[(mIsOut ? front : rear) * mFrameSize] : NULL;
    buffer->mNonContig = availToServer - part1;
    // After flush(), allow releaseBuffer() on a previously obtained buffer;
    // see "Acknowledge any pending flush()" in audioflinger/Tracks.cpp.
    if (!ackFlush) {
        mUnreleased = part1;
    }
    return part1 > 0 ? NO_ERROR : WOULD_BLOCK;
    }
no_init:
    buffer->mFrameCount = 0;
    buffer->mRaw = NULL;
    buffer->mNonContig = 0;
    mUnreleased = 0;
    return NO_INIT;
}

void ServerProxy::releaseBuffer(Buffer* buffer)
{
    LOG_ALWAYS_FATAL_IF(buffer == NULL);
    size_t stepCount = buffer->mFrameCount;
    if (stepCount == 0 || mIsShutdown) {
        // prevent accidental re-use of buffer
        buffer->mFrameCount = 0;
        buffer->mRaw = NULL;
        buffer->mNonContig = 0;
        return;
    }
    LOG_ALWAYS_FATAL_IF(!(stepCount <= mUnreleased && mUnreleased <= mFrameCount));
    mUnreleased -= stepCount;
    audio_track_cblk_t* cblk = mCblk;
    if (mIsOut) {
        int32_t front = cblk->u.mStreaming.mFront;
        android_atomic_release_store(stepCount + front, &cblk->u.mStreaming.mFront);
    } else {
        int32_t rear = cblk->u.mStreaming.mRear;
        android_atomic_release_store(stepCount + rear, &cblk->u.mStreaming.mRear);
    }

    cblk->mServer += stepCount;

    size_t half = mFrameCount / 2;
    if (half == 0) {
        half = 1;
    }
    size_t minimum = (size_t) cblk->mMinimum;
    if (minimum == 0) {
        minimum = mIsOut ? half : 1;
    } else if (minimum > half) {
        minimum = half;
    }
    // FIXME AudioRecord wakeup needs to be optimized; it currently wakes up client every time
    if (!mIsOut || (mAvailToClient + stepCount >= minimum)) {
        ALOGV("mAvailToClient=%zu stepCount=%zu minimum=%zu", mAvailToClient, stepCount, minimum);
        int32_t old = android_atomic_or(CBLK_FUTEX_WAKE, &cblk->mFutex);
        if (!(old & CBLK_FUTEX_WAKE)) {
            (void) syscall(__NR_futex, &cblk->mFutex,
                    mClientInServer ? FUTEX_WAKE_PRIVATE : FUTEX_WAKE, 1);
        }
    }

    buffer->mFrameCount = 0;
    buffer->mRaw = NULL;
    buffer->mNonContig = 0;
}

// ---------------------------------------------------------------------------

size_t AudioTrackServerProxy::framesReady()
{
    LOG_ALWAYS_FATAL_IF(!mIsOut);

    if (mIsShutdown) {
        return 0;
    }
    audio_track_cblk_t* cblk = mCblk;

    int32_t flush = cblk->u.mStreaming.mFlush;
    if (flush != mFlush) {
        // FIXME should return an accurate value, but over-estimate is better than under-estimate
        return mFrameCount;
    }
    // the acquire might not be necessary since not doing a subsequent read
    int32_t rear = android_atomic_acquire_load(&cblk->u.mStreaming.mRear);
    ssize_t filled = rear - cblk->u.mStreaming.mFront;
    // pipe should not already be overfull
    if (!(0 <= filled && (size_t) filled <= mFrameCount)) {
        ALOGE("Shared memory control block is corrupt (filled=%zd); shutting down", filled);
        mIsShutdown = true;
        return 0;
    }
    //  cache this value for later use by obtainBuffer(), with added barrier
    //  and racy if called by normal mixer thread
    // ignores flush(), so framesReady() may report a larger mFrameCount than obtainBuffer()
    return filled;
}

bool  AudioTrackServerProxy::setStreamEndDone() {
    audio_track_cblk_t* cblk = mCblk;
    bool old =
            (android_atomic_or(CBLK_STREAM_END_DONE, &cblk->mFlags) & CBLK_STREAM_END_DONE) != 0;
    if (!old) {
        (void) syscall(__NR_futex, &cblk->mFutex, mClientInServer ? FUTEX_WAKE_PRIVATE : FUTEX_WAKE,
                1);
    }
    return old;
}

void AudioTrackServerProxy::tallyUnderrunFrames(uint32_t frameCount)
{
    audio_track_cblk_t* cblk = mCblk;
    cblk->u.mStreaming.mUnderrunFrames += frameCount;

    // FIXME also wake futex so that underrun is noticed more quickly
    (void) android_atomic_or(CBLK_UNDERRUN, &cblk->mFlags);
}

// ---------------------------------------------------------------------------

StaticAudioTrackServerProxy::StaticAudioTrackServerProxy(audio_track_cblk_t* cblk, void *buffers,
        size_t frameCount, size_t frameSize)
    : AudioTrackServerProxy(cblk, buffers, frameCount, frameSize),
      mObserver(&cblk->u.mStatic.mSingleStateQueue), mPosition(0),
      mFramesReadySafe(frameCount), mFramesReady(frameCount),
      mFramesReadyIsCalledByMultipleThreads(false)
{
    mState.mLoopStart = 0;
    mState.mLoopEnd = 0;
    mState.mLoopCount = 0;
}

void StaticAudioTrackServerProxy::framesReadyIsCalledByMultipleThreads()
{
    mFramesReadyIsCalledByMultipleThreads = true;
}

size_t StaticAudioTrackServerProxy::framesReady()
{
    // Can't call pollPosition() from multiple threads.
    if (!mFramesReadyIsCalledByMultipleThreads) {
        (void) pollPosition();
    }
    return mFramesReadySafe;
}

ssize_t StaticAudioTrackServerProxy::pollPosition()
{
    size_t position = mPosition;
    StaticAudioTrackState state;
    if (mObserver.poll(state)) {
        bool valid = false;
        size_t loopStart = state.mLoopStart;
        size_t loopEnd = state.mLoopEnd;
        if (state.mLoopCount == 0) {
            if (loopStart > mFrameCount) {
                loopStart = mFrameCount;
            }
            // ignore loopEnd
            mPosition = position = loopStart;
            mFramesReady = mFrameCount - mPosition;
            mState.mLoopCount = 0;
            valid = true;
        } else if (state.mLoopCount >= -1) {
            if (loopStart < loopEnd && loopEnd <= mFrameCount &&
                    loopEnd - loopStart >= MIN_LOOP) {
                // If the current position is greater than the end of the loop
                // we "wrap" to the loop start. This might cause an audible pop.
                if (position >= loopEnd) {
                    mPosition = position = loopStart;
                }
                if (state.mLoopCount == -1) {
                    mFramesReady = INT64_MAX;
                } else {
                    // mFramesReady is 64 bits to handle the effective number of frames
                    // that the static audio track contains, including loops.
                    // TODO: Later consider fixing overflow, but does not seem needed now
                    // as will not overflow if loopStart and loopEnd are Java "ints".
                    mFramesReady = int64_t(state.mLoopCount) * (loopEnd - loopStart)
                            + mFrameCount - mPosition;
                }
                mState = state;
                valid = true;
            }
        }
        if (!valid || mPosition > mFrameCount) {
            ALOGE("%s client pushed an invalid state, shutting down", __func__);
            mIsShutdown = true;
            return (ssize_t) NO_INIT;
        }
        mFramesReadySafe = clampToSize(mFramesReady);
        // This may overflow, but client is not supposed to rely on it
        mCblk->u.mStatic.mBufferPosition = (uint32_t) position;
    }
    return (ssize_t) position;
}

status_t StaticAudioTrackServerProxy::obtainBuffer(Buffer* buffer, bool ackFlush __unused)
{
    if (mIsShutdown) {
        buffer->mFrameCount = 0;
        buffer->mRaw = NULL;
        buffer->mNonContig = 0;
        mUnreleased = 0;
        return NO_INIT;
    }
    ssize_t positionOrStatus = pollPosition();
    if (positionOrStatus < 0) {
        buffer->mFrameCount = 0;
        buffer->mRaw = NULL;
        buffer->mNonContig = 0;
        mUnreleased = 0;
        return (status_t) positionOrStatus;
    }
    size_t position = (size_t) positionOrStatus;
    size_t end = mState.mLoopCount != 0 ? mState.mLoopEnd : mFrameCount;
    size_t avail;
    if (position < end) {
        avail = end - position;
        size_t wanted = buffer->mFrameCount;
        if (avail < wanted) {
            buffer->mFrameCount = avail;
        } else {
            avail = wanted;
        }
        buffer->mRaw = &((char *) mBuffers)[position * mFrameSize];
    } else {
        avail = 0;
        buffer->mFrameCount = 0;
        buffer->mRaw = NULL;
    }
    // As mFramesReady is the total remaining frames in the static audio track,
    // it is always larger or equal to avail.
    LOG_ALWAYS_FATAL_IF(mFramesReady < avail);
    buffer->mNonContig = mFramesReady == INT64_MAX ? SIZE_MAX : clampToSize(mFramesReady - avail);
    mUnreleased = avail;
    return NO_ERROR;
}

void StaticAudioTrackServerProxy::releaseBuffer(Buffer* buffer)
{
    size_t stepCount = buffer->mFrameCount;
    LOG_ALWAYS_FATAL_IF(!(stepCount <= mFramesReady));
    LOG_ALWAYS_FATAL_IF(!(stepCount <= mUnreleased));
    if (stepCount == 0) {
        // prevent accidental re-use of buffer
        buffer->mRaw = NULL;
        buffer->mNonContig = 0;
        return;
    }
    mUnreleased -= stepCount;
    audio_track_cblk_t* cblk = mCblk;
    size_t position = mPosition;
    size_t newPosition = position + stepCount;
    int32_t setFlags = 0;
    if (!(position <= newPosition && newPosition <= mFrameCount)) {
        ALOGW("%s newPosition %zu outside [%zu, %zu]", __func__, newPosition, position, mFrameCount);
        newPosition = mFrameCount;
    } else if (mState.mLoopCount != 0 && newPosition == mState.mLoopEnd) {
        newPosition = mState.mLoopStart;
        if (mState.mLoopCount == -1 || --mState.mLoopCount != 0) {
            setFlags = CBLK_LOOP_CYCLE;
        } else {
            setFlags = CBLK_LOOP_FINAL;
        }
    }
    if (newPosition == mFrameCount) {
        setFlags |= CBLK_BUFFER_END;
    }
    mPosition = newPosition;
    if (mFramesReady != INT64_MAX) {
        mFramesReady -= stepCount;
    }
    mFramesReadySafe = clampToSize(mFramesReady);

    cblk->mServer += stepCount;
    // This may overflow, but client is not supposed to rely on it
    cblk->u.mStatic.mBufferPosition = (uint32_t) newPosition;
    if (setFlags != 0) {
        (void) android_atomic_or(setFlags, &cblk->mFlags);
        // this would be a good place to wake a futex
    }

    buffer->mFrameCount = 0;
    buffer->mRaw = NULL;
    buffer->mNonContig = 0;
}

void StaticAudioTrackServerProxy::tallyUnderrunFrames(uint32_t frameCount __unused)
{
    // Unlike AudioTrackServerProxy::tallyUnderrunFrames() used for streaming tracks,
    // we don't have a location to count underrun frames.  The underrun frame counter
    // only exists in AudioTrackSharedStreaming.  Fortunately, underruns are not
    // possible for static buffer tracks other than at end of buffer, so this is not a loss.

    // FIXME also wake futex so that underrun is noticed more quickly
    (void) android_atomic_or(CBLK_UNDERRUN, &mCblk->mFlags);
}

// ---------------------------------------------------------------------------

}   // namespace android
