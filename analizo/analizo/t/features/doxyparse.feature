Feature: doxyparse extractor external tool
  As a Analizo developer
  I want to guarantee that doxyparse deal with any source code
  To provide reliability for Analizo users

  Scenario: don't die parsing MCLinker.cpp from android 5.1.1
    Given I am in t/samples/android-framework/android-5.1.1_r38
    When I run "analizo metrics ."
    Then the exit status must be 0

  Scenario: don't duplicate YAML keys parsing AudioTrackShared.cpp from android 5.1.1
    Given I am in t/samples/android-framework/android-5.1.1_r38
    When I run "analizo metrics ."
    Then analizo must not emit a warning matching "YAML_LOAD_WARN_DUPLICATE_KEY"

  Scenario: don't abort parsing mlpack 3.0.0
    Given I am in t/samples/mlpack-3.0.0
    When I run "analizo metrics ."
    Then analizo must not emit a warning matching "Aborted"
    And the exit status must be 0

  Scenario: don't die parsing kdelibs warning about unknown escape character
    Given I am in t/samples/kdelibs
    When I run "analizo metrics ."
    Then analizo must not emit a warning matching "Error"
    And the exit status must be 0

  Scenario: don't die parsing mod_suexec.h from http 2.4.38
    Given I am in t/samples/httpd-2.4.38
    When I run "analizo metrics ."
    Then analizo must not emit a warning matching "Not a HASH reference"
    And the exit status must be 0

  Scenario: allow dot on module filename
    Given I am in t/samples/sample_basic/c
    When I run "analizo metrics ."
    Then analizo must report that file module1.c declares module module1
