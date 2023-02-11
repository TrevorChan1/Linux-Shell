rm f3
mkdir trash_folder
mv f1 f2 f4 list_of_folders.txt sorted_folders.txt trash_folder
ls -a trash_folder | sort | head -3 > test2out.txt
