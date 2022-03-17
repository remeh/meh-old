#!/bin/sh
src=$1
convert "$src" -resize '!16x16' -unsharp 1x4 "icon_16x16.png"
convert "$src" -resize '!32x32' -unsharp 1x4 "icon_16x16@2x.png"
convert "$src" -resize '!32x32' -unsharp 1x4 "icon_32x32.png"
convert "$src" -resize '!64x64' -unsharp 1x4 "icon_32x32@2x.png"
convert "$src" -resize '!128x128' -unsharp 1x4 "icon_128x128.png"
convert "$src" -resize '!256x256' -unsharp 1x4 "icon_128x128@2x.png"
convert "$src" -resize '!256x256' -unsharp 1x4 "icon_256x256.png"
convert "$src" -resize '!512x512' -unsharp 1x4 "icon_256x256@2x.png"
convert "$src" -resize '!512x512' -unsharp 1x4 "icon_512x512.png"
convert "$src" -resize '!1024x1024' -unsharp 1x4 "icon_512x512@2x.png"
