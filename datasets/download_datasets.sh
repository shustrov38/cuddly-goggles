#!/bin/bash

file_id="1fIPkocdNHxdXgLP1uNYRVgltVUXZQvzm"
link="https://drive.google.com/uc?export=download&id="$file_id"&confirm=yes"

path=$(dirname $(realpath $0))
name_zip=datasets.zip

wget -O $path/$name_zip $link