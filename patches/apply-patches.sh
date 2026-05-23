#!/bin/bash

# Script to apply patches to third_party/skia
# Usage: ./patches/apply-patches.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SKIA_DIR="$PROJECT_ROOT/third_party/skia"
PATCHES_DIR="$SCRIPT_DIR/skia"

echo "🔧 Applying patches to third_party/skia..."

if [ ! -d "$SKIA_DIR" ]; then
    echo "❌ Error: third_party/skia directory not found"
    echo "   Make sure the submodule is initialized: git submodule update --init"
    exit 1
fi

cd "$SKIA_DIR"

# Check if there are any patches to apply
if [ ! -d "$PATCHES_DIR" ] || [ -z "$(ls -A "$PATCHES_DIR"/*.patch 2>/dev/null)" ]; then
    echo "ℹ️  No patches found in $PATCHES_DIR"
    exit 0
fi

# Apply each patch
for patch_file in "$PATCHES_DIR"/*.patch; do
    if [ -f "$patch_file" ]; then
        patch_name=$(basename "$patch_file")
        echo "📝 Applying $patch_name..."
        
        # Try to apply the patch
        if git apply --check "$patch_file" 2>/dev/null; then
            git apply "$patch_file"
            echo "   ✅ Applied successfully"
        else
            # Check if already applied
            if git apply --reverse --check "$patch_file" 2>/dev/null; then
                echo "   ℹ️  Already applied, skipping"
            else
                echo "   ⚠️  Warning: Patch may have conflicts or already partially applied"
                echo "   Attempting to apply with 3-way merge..."
                if git apply --3way "$patch_file"; then
                    echo "   ✅ Applied with 3-way merge"
                else
                    echo "   ❌ Failed to apply patch. Please review manually."
                    exit 1
                fi
            fi
        fi
    fi
done

echo "✅ All patches applied successfully!"

