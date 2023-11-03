#!/bin/bash

#Take a photo and place it in the image library
libcamera-still -o imageLibrary/imageBig.jpg

#Crop image down into 32x24 <255 byte images: 
#1 Reduce image size, \! makes the resize ignore aspect ratio and hence actually will size to the specified dimensions
convert imageLibrary/imageBig.jpg -resize 512x384\! imageLibrary/imageLittle.jpg

#2. First make image gray
convert imageLibrary/imageLittle.jpg -grayscale Rec709Luma imageLibrary/imageGray.jpg

#3. then reduce quality
convert -strip -interlace Plane -gaussian-blur 0.05 -quality 50% imageLibrary/imageGray.jpg imageLibrary/imageBlur.jpg
			
#4. then crop (use %04d so files are in order)
convert imageLibrary/imageBlur.jpg -crop 32x24\! imageSplit/imageBit_%04d.jpg

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