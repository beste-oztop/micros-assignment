#!/bin/bash
# filepath: copy_files.sh

SOURCE_DIR="/media/sf_shared_folder/micros-assignment"
DEST_DIR="."

# Check if source directory exists
if [ ! -d "$SOURCE_DIR" ]; then
    echo "Error: Source directory $SOURCE_DIR does not exist"
    exit 1
fi

# Copy files (not directories) from source to destination
for file in "$SOURCE_DIR"/*; do
    # Check if it's a regular file (not directory)
    if [ -f "$file" ]; then
        filename=$(basename "$file")
        # Skip todo.md
        if [ "$filename" != "todo.md" ] && [ "$filename" != "TODO.md" ]; then
            echo "Copying $filename..."
            cp "$file" "$DEST_DIR/"
        else
            echo "Skipping $filename"
        fi
    fi
done

echo "Done!"