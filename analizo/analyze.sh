analizo metrics ../../libevent/ --exclude ../../libevent/build > libevent_matrics.yaml
python metrics_converter.py ./libevent_matrics.yaml libevent_matrics.md