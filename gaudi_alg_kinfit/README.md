# Introduction

In this tutorial we are going to explor how to run a kinematic fit inside Gaudi.

## Goals
- Show how to combine existing SW from LC stack with Gaudi
- Potentially uncover a few hickups that one encounters when doing so


### Potential pitfals
- Using e.g. `k4FWCore::Transformer` instead of only `Transformer` in the constructor initializer list
  - will yield a compiler error with *no matching function call to ...* or *expected class-name before '(' token*
