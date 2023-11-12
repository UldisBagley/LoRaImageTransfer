#!/bin/bash

#Take a photo and place it in the image library
libcamera-still -o imageLibrary/imageBig.jpg

# Rotate because camera is upside down
convert imageLibrary/imageBig.jpg -rotate 180 imageLibrary/imageBig.jpg

#convert to WebP type
convert imageLibrary/imageBig.jpg imageLibrary/imageBig.WebP

#Crop image down into 32x24 <255 byte images: 
#1 Reduce image size, \! makes the resize ignore aspect ratio and hence actually will size to the specified dimensions
convert imageLibrary/imageBig.WebP -resize 512x384\! imageLibrary/imageLittle.WebP

#2. First make image gray
convert imageLibrary/imageLittle.WebP -grayscale Rec709Luma imageLibrary/imageGray.WebP

#3. then reduce quality
convert -strip -interlace Plane imageLibrary/imageGray.WebP imageLibrary/imageBlur.WebP
			
#4. then crop (use %04d so files are in order)
convert  imageLibrary/imageBlur.WebP -crop 32x24\! imageSplit/imageBit_%04d.WebP

# 5 if any tiles are greater than or equal to 250 bytes, reduce quality
dir="imageSplit"
for f in "$dir"/*; do
    size=$(stat -c '%s' "$f")
    if [[ $size -ge 250 ]]; then
        convert "$f" +level 40%,40%  -alpha off "$f"
        echo "$f size reduced"
    fi
done

# dir="imageSplit"
# x=0
# while [ $x -eq 0 ]; do
#     for f in "$dir"/*;do
#         size=$( stat -c '%s' $f )
#         if [ size -ge 250 ]
#         then 
#             convert -strip -interlace Plane -gaussian-blur 0.05 -quality 50% $f $f
#             x=1
#             break
#         fi
#     done
# done

#Now run the program
sudo ../build/./LoRaImageTransfer