#!/bin/bash

# Loop through all PNG files in the current directory
for file in *_calibration*.png; do
    # Skip if no files found
    [ -e "$file" ] || continue
    
    # Extract the datetime prefix (20241024_163609)
    datetime_prefix=$(echo "$file" | cut -d'_' -f1,2)
    
    # Extract the remaining part of the filename after datetime
    remaining_name=$(echo "$file" | cut -d'_' -f3-)
    
    # Create directory if it doesn't exist
    mkdir -p "$datetime_prefix"
    
    # Move and rename the file
    mv "$file" "$datetime_prefix/$remaining_name"
    
    echo "Moved $file to $datetime_prefix/$remaining_name"
done

# Print the final structure
echo -e "\nFinal structure:"
ls -R
