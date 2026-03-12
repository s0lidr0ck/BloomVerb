param(
    [ValidateSet("full", "tests-only")]
    [string]$Mode = "full",
    [string]$Preset = ""
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($Preset)) {
    if ($Mode -eq "tests-only") {
        $Preset = "windows-tests"
    }
    else {
        $Preset = "windows-release"
    }
}

cmake --preset $Preset

if ($Mode -eq "tests-only") {
    cmake --build --preset "$Preset-build" --target `
        BloomVerbEngineTests `
        BloomVerbEngineInvarianceTests `
        BloomVerbStateTests `
        BloomVerbPresetRecallTests `
        BloomVerbFDNRegressionTests `
        BloomVerbFreezeAutomationStressTests `
        BloomVerbPerformanceGateTests `
        BloomVerbMonoFoldDownTests
}
else {
    cmake --build --preset "$Preset-build"
}

ctest --preset "$Preset-test"
