# Testing your shell
# Place this in the same directory as your compiled shell "myshell", and run this script with `bash run_test.sh`
# It will generate a test folder, output test1.sh and test2.sh, which contian a list of shell commands to be run
# This script will redirect the test script into you shell (myshell) and attempt to run it. (Your shell must exit when it 
# recieves CTRL+D(End Of File)). Then the script then tests that the results are correct.
# You can use this as a template to make your own tests
# You should be able to pass any "simple" shell script (only containing the features you implemented) into myshell by typing:
# ./myshell < script.sh
# '-n' is just to hide the "my_shell$" prompt

rm -rf shell_test_folder
mkdir shell_test_folder
cd shell_test_folder


# test 1: make files, ls sort
echo "touch f1 f2 f3 f4
ls > list_of_folders.txt
sort < list_of_folders.txt > sorted_folders.txt" > test1.sh

# run shell
../myshell -n < test1.sh

# check results
# if statement compares the txt file to expected output from standard bash shell
if [ "f1
f2
f3
f4
list_of_folders.txt
sorted_folders.txt
test1.sh" == "$(cat sorted_folders.txt)" ] ;
then
    echo "test 1 pass";
else
    echo "test 1 fail";
fi



# test 2
echo "rm f3
mkdir trash_folder
mv f1 f2 f4 list_of_folders.txt sorted_folders.txt trash_folder
ls -a trash_folder | sort | head -3 > test2out.txt" > test2.sh

# run shell
../myshell -n < test2.sh >  /dev/null 2>&1

# check results
if [ "$(cat test2out.txt)" == ".
..
f1" ] && [ -e trash_folder/f1 ] && [ -e trash_folder/list_of_folders.txt ] && [ -e trash_folder/sorted_folders.txt ] # check if moved files exist
then
    echo "test 2 pass"
else
    echo "test 2 fail"
fi