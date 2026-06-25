# Testing

## Overview

The project includes a comprehensive test suite that validates parameter scaling, DSP engine behavior, and system invariants. Tests are implemented using JUCE's built-in `UnitTest` framework.

## Test Structure

Tests are located in `tests/TestMain.cpp` and organized into two tiers:

### Tier 1: Parameter Scaling Tests

These tests validate the `toEngineValue()` function and parameter table invariants.

**Test Class**: `ParameterScalingTests`

**Test Cases**:

1. **Table Invariants**
   - Verifies exactly 13 parameters exist
   - Checks all parameter IDs are unique
   - Validates min <= max for all parameters
   - Ensures default values are within valid ranges

2. **Linear Passthrough**
   - Tests `reverb_dry_wet` (linear scaling)
   - Verifies: `toEngineValue(param, 0.5) == 0.5`
   - Verifies: `toEngineValue(param, 0.0) == 0.0`
   - Verifies: `toEngineValue(param, 1.0) == 1.0`

3. **Exponential Scaling**
   - Tests `reverb_decay_time` (exponential, 2^x)
   - Verifies: `toEngineValue(param, 0.0) == 1.0` (2^0)
   - Verifies: `toEngineValue(param, 6.0) == 64.0` (2^6)
   - Verifies: `toEngineValue(param, -6.0) == 0.015625` (2^-6)
   - Verifies clamping: `toEngineValue(param, 99.0) == 64.0` (max)
   
   - Tests `reverb_chorus_frequency` (exponential, 2^x)
   - Verifies: `toEngineValue(param, 0.0) == 1.0`
   - Verifies: `toEngineValue(param, 3.0) == 8.0` (2^3)

4. **Quadratic Scaling**
   - Tests `reverb_chorus_amount` (quadratic, x²)
   - Verifies: `toEngineValue(param, 0.5) == 0.25` (0.5²)
   - Verifies: `toEngineValue(param, 1.0) == 1.0` (1.0²)
   - Verifies clamping: `toEngineValue(param, -0.3) == 0.0` (clamped to 0)

### Tier 2: Engine Smoke Tests

These tests validate the DSP engine's runtime behavior.

**Test Class**: `EngineSmokeTests`

**Test Cases**:

1. **Reverb Always On: Finite, Bounded Output**
   - Initializes engine with default parameters
   - Processes 200 blocks of white noise
   - Verifies all output samples are finite (no NaN/Inf)
   - Verifies output magnitude stays below 50.0 (no explosion)

2. **Reset Clears Tails Toward Silence**
   - Processes one block of noise to populate delay lines
   - Calls `engine.reset()`
   - Processes 50 blocks of silence
   - Verifies all samples are finite
   - Verifies output magnitude decays below 1e-3 (tail cleared)

## Running Tests

### Build Test Executable

```bash
# Configure and build
cmake -B build
cmake --build build --config Release --target VialReverb_Tests
```

### Run Tests

```bash
# Execute the test binary
./build/VialReverb_artefacts/Release/VialReverb_Tests
```

### Expected Output

```
  [PASS] Parameter scaling / vfx  (12 checks)
  [PASS] Engine smoke / vfx  (402 checks)
ALL TESTS PASSED — 2 sections, 414 checks, 0 failures
```

If tests fail, you'll see:

```
  [FAIL] Parameter scaling / vfx  (8 passes, 4 failures)
TESTS FAILED — 2 sections, 410 checks, 4 failures
```

## Test Coverage

### What's Tested

✅ **Parameter Scaling**
- All scaling functions (Linear, Exponential, Quadratic)
- Boundary conditions (min, max, default)
- Clamping behavior

✅ **DSP Engine**
- Finite output (no NaN/Inf)
- Stable feedback (no explosion)
- Reset behavior (tail decay)
- Always-on processing

### What's Not Tested

❌ **Audio Quality**
- Subjective sound quality
- Frequency response accuracy
- Stereo imaging

❌ **Performance**
- CPU usage benchmarks
- Memory allocation patterns
- Real-time safety

❌ **Edge Cases**
- Extreme parameter combinations
- Sample rate changes during processing
- Buffer size changes during processing

## Adding New Tests

### Step 1: Create Test Class

```cpp
class MyNewTests : public juce::UnitTest {
public:
    MyNewTests() : juce::UnitTest("My Test Category", "vfx") {}

    void runTest() override {
        beginTest("test case name");
        {
            // Test code here
            expect(condition, "failure message");
            expectEquals(actual, expected, "failure message");
            expectWithinAbsoluteError(actual, expected, tolerance, "message");
        }
    }
};
```

### Step 2: Register Test

```cpp
static MyNewTests myNewTests;
```

### Step 3: Build and Run

```bash
cmake --build build --target VialReverb_Tests
./build/VialReverb_artefacts/Release/VialReverb_Tests
```

## Test Best Practices

### 1. Use Descriptive Test Names

```cpp
beginTest("exponential = 2^x, clamped to [min,max]");
```

### 2. Provide Clear Failure Messages

```cpp
expect(param != nullptr, "param not found");
expectWithinAbsoluteError(actual, expected, 1e-6f, "scaling mismatch");
```

### 3. Test Boundary Conditions

```cpp
// Test min, max, and default values
expectEquals(toEngineValue(param, 0.0f), 0.0f, 1e-6f);
expectEquals(toEngineValue(param, 1.0f), 1.0f, 1e-6f);
```

### 4. Use Appropriate Tolerances

```cpp
// Floating-point comparisons need tolerance
expectWithinAbsoluteError(actual, expected, 1e-5f, "message");
```

### 5. Initialize State Properly

```cpp
ReverbEngine engine;
engine.prepare(44100, 512);
engine.setRawParameters(defaults());
```

## Continuous Integration

### GitHub Actions Example

```yaml
name: Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Install dependencies
        run: sudo apt-get update && sudo apt-get install -y libasound2-dev libx11-dev
        
      - name: Configure
        run: cmake -B build
        
      - name: Build tests
        run: cmake --build build --target VialReverb_Tests
        
      - name: Run tests
        run: ./build/VialReverb_artefacts/Release/VialReverb_Tests
```

## Debugging Failed Tests

### Step 1: Identify Failing Test

Look for `[FAIL]` in the output:

```
[FAIL] Parameter scaling / vfx  (8 passes, 4 failures)
```

### Step 2: Add Debug Output

Temporarily add print statements:

```cpp
beginTest("debug test");
{
    float result = toEngineValue(param, 0.5f);
    std::cout << "Result: " << result << std::endl;
    expectEquals(result, 0.5f, 1e-6f);
}
```

### Step 3: Check Parameter Values

```cpp
const auto* param = findParam("reverb_dry_wet");
if (param) {
    std::cout << "Param: " << param->id 
              << " min=" << param->min 
              << " max=" << param->max 
              << " default=" << param->defaultValue << std::endl;
}
```

### Step 4: Verify Scaling Logic

Check `ReverbParameters.h` for the scaling implementation:

```cpp
case Scaling::Linear:
    return value * (param.max - param.min) + param.min;
```

## Test Data

### Helper Functions

```cpp
// Create default parameter array
static std::array<float, kParams.size()> defaults() {
    std::array<float, kParams.size()> raw {};
    for (size_t i = 0; i < kParams.size(); ++i)
        raw[i] = kParams[i].defaultValue;
    return raw;
}

// Check if buffer contains only finite values
static bool allFinite(const juce::AudioBuffer<float>& b) {
    for (int c = 0; c < b.getNumChannels(); ++c)
        for (int i = 0; i < b.getNumSamples(); ++i)
            if (!std::isfinite(b.getSample(c, i)))
                return false;
    return true;
}

// Fill buffer with white noise
void fillNoise(juce::AudioBuffer<float>& b, juce::Random& r) {
    for (int c = 0; c < b.getNumChannels(); ++c)
        for (int i = 0; i < b.getNumSamples(); ++i)
            b.setSample(c, i, r.nextFloat() * 2.0f - 1.0f);
}
```

## Future Test Improvements

### Potential Additions

1. **Frequency Response Tests**
   - Sweep sine wave through reverb
   - Measure frequency response
   - Verify filter cutoffs match expected values

2. **Decay Time Tests**
   - Process impulse
   - Measure actual decay time
   - Compare to parameter setting

3. **Stereo Tests**
   - Process mono input
   - Verify stereo output characteristics
   - Test stereo width parameter

4. **Performance Tests**
   - Measure CPU usage across parameter ranges
   - Identify performance bottlenecks
   - Set performance budgets

5. **Stress Tests**
   - Rapid parameter changes
   - Sample rate changes
   - Buffer size changes
