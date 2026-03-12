#!/usr/bin/env bash
set -euo pipefail

MODE="full"
PRESET=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --mode)
      MODE="$2"
      shift 2
      ;;
    --preset)
      PRESET="$2"
      shift 2
      ;;
    *)
      echo "Unknown argument: $1" >&2
      exit 1
      ;;
  esac
done

if [[ -z "$PRESET" ]]; then
  if [[ "$MODE" == "tests-only" ]]; then
    PRESET="linux-tests"
  else
    PRESET="linux-release"
  fi
fi

cmake --preset "$PRESET"

if [[ "$MODE" == "tests-only" ]]; then
  cmake --build --preset "${PRESET}-build" --target \
    BloomVerbEngineTests \
    BloomVerbEngineInvarianceTests \
    BloomVerbStateTests \
    BloomVerbPresetRecallTests \
    BloomVerbFDNRegressionTests \
    BloomVerbFreezeAutomationStressTests \
    BloomVerbPerformanceGateTests \
    BloomVerbMonoFoldDownTests
else
  cmake --build --preset "${PRESET}-build"
fi

ctest --preset "${PRESET}-test"
