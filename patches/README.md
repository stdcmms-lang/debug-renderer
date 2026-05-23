# Skia Patches

This directory contains patches applied to the `third_party/skia` submodule.

## How to Patch Skia

### Method 1: Create a Patch from Current Changes

If you've made changes directly in the `third_party/skia` directory:

```bash
# 1. Navigate to the skia submodule
cd third_party/skia

# 2. Create a patch file from your changes
git diff > ../../patches/skia/your-patch-name.patch

# 3. Reset the submodule to clean state (optional, if you want to test the patch)
git restore .

# 4. Apply the patch using the script
cd ../..
./patches/apply-patches.sh
```

### Method 2: Create a Patch from a Specific Commit

If you've committed changes in the submodule:

```bash
cd third_party/skia
git format-patch -1 HEAD --stdout > ../../patches/skia/your-patch-name.patch
```

### Method 3: Manual Patch Creation

1. Make your changes in `third_party/skia`
2. Generate the patch:
   ```bash
   cd third_party/skia
   git diff > ../../patches/skia/your-patch-name.patch
   ```
3. Review the patch file
4. Reset the submodule:
   ```bash
   git restore .
   ```
5. Test applying the patch:
   ```bash
   cd ../..
   ./patches/apply-patches.sh
   ```

## Applying Patches

To apply all patches:

```bash
./patches/apply-patches.sh
```

This script will:

- Check if the submodule exists
- Apply each `.patch` file in `patches/skia/`
- Handle already-applied patches gracefully
- Use 3-way merge if there are conflicts

## Updating the Submodule

When updating the Skia submodule to a new version:

1. Update the submodule:

   ```bash
   cd third_party/skia
   git fetch
   git checkout <new-commit-or-branch>
   cd ../..
   ```

2. Try to apply patches:

   ```bash
   ./patches/apply-patches.sh
   ```

3. If patches fail, you may need to:
   - Update the patch files manually
   - Create new patch files for the new version
   - Remove patches that are no longer needed (if changes were upstreamed)

## Best Practices

1. **Name patches descriptively**: Use names like `textstyle-fix.patch`, `font-family-fix.patch`, etc.
2. **One patch per feature/fix**: Don't mix unrelated changes in one patch
3. **Document patches**: Add comments in patch files or this README explaining what each patch does
4. **Test after applying**: Always test that your code still works after applying patches
5. **Version control patches**: Commit patch files to your repository so they're tracked

## Current Patches

- `textstyle-fix.patch`: Fixes static initialization order issue in TextStyle by converting `kDefaultFontFamilies` to a function
