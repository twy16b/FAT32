# OS-Project3

## Division of Labor

- exit: Tyler
- info: Tyler
- size: Chelese
- ls: Chelese
- cd: Tyler
- creat: Chelese
- mkdir: Kevin
- mv: Tyler
- open: Tyler
- close: Tyler
- read: Tyler
- write: Kevin
- rm: Tyler
- cp: Chelese

## Commands

## exit

Exits the program and frees all allocated resources

## info

Prints out:

- bytes per sector
- sectors per cluster
- reseverd sector count
- number of FATs
- total sectors
- FATsize
- root cluster

## size FILENAME

Prints out the filesize of the file in the current directory with the name FILENAME.

## ls DIRNAME

Prints out all entries within DIRNAME found in the current directory.

If no DIRNAME is given, it prints out all the entries in the current directory.

## cd DIRNAME

Changes the current directory to DIRNAME and updates the PWD variable.

If no DIRNAME is given, it returns to the root directory.

## creat FILENAME

Creates a new file in the current directory named FILENAME.

## mkdir DIRNAME

Creates a new directory in the current directory called DIRNAME.

## mv FROM TO

If TO is a directory in the current directory, it will move the FROM file into that directory.

If TO is not found, FROM will be renamed.

## open FILENAME MODE

Opens the file in the current directory named FILENAME with MODE denoting whether it is read-only, write-only, or both.

## close FILENAME

Closes the file in the current directory named FILENAME if it is open.

## read FILENAME OFFSET SIZE

If the file has been opened in read mode, it prints out SIZE bytes starting at OFFSET.

## write FILENAME OFFSET SIZE "STRING"

If the file has been opened in write mode, it writes "STRING" of size BYTES starting at OFFSET.

If OFFSET + SIZE is greater than the current filesize, it will increase the file size for that file's entry.

## rm FILENAME

Removes the file in the current directory called FILENAME and marks all associated FAT entries as empty.

Also removed the long name entry if applicable.

## cp FILENAME TO

Creates a copy of the file in the current directory called FILENAME in the directory called TO.

If TO is not given, create a copy in the current directory called TO.

# Things to Note

All file and directory names are converted to uppercase as per the FAT32 specifications, including the user input before being compared to the entry names. Names can be no longer than 8 characters.

# Known Bugs

If the last file in a cluster is removed / moved, the cluster remains allocated in the FAT.

If a file is removed / moved and isn't the last entry in the directory, it is marked with value 229 as per the FAT32 specifications. However, if the last file is then removed / moved, that entry will be marked with 0 while the previous entry will remain marked with 229 even if it is now the last entry in that directory.
