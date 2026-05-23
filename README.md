# Debug Web Renderer

Fix the **Skia-based renderer backend** in this repository.

The implementation is intentionally buggy. Your goal is to make the renderer correct enough for the provided test suite to pass.

This is a timed exercise: use AI to help you finish the implementation in the time window.

## Run unit tests

```bash
make test
```

Each test renders a PNG image and the image is compared to a fixture. A failed test will write the rendered result to fixture/xxx_last_run.png. Do **NOT** change the original fixture/xxx.png otherwise you will lose the ground truth.

## How you will be evaluated

You will be evaluated by how many renderer tests pass.
