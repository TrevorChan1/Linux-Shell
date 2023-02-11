rm f3
mkdir trash_folder
mv f1 f2 f4 list_of_folders.txt sorted_folders.txt test1.sh test2.sh test3.sh test4.sh test5.sh test6.sh test7.sh test7.sh test2out.txt test3out.txt test4out.txt test5out.txt test6out.txt test7out.txt test8out.txt trash_folder
ls -a trash_folder | sort | head -3 > test2out.txt
