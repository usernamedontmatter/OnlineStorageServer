# Command-line interface
```
./OnlineStorage --root_path path_to_root_directory --address server_address --port server_port
```
* path_to_root_directory - path to directory of file storage
* server_address - web address of server
* server_port - port of server

If one of the parameters(except path_to_root_directory) don't exists default value may be used

# Web interface
```
show_files path
```
* path - path to viewed directory
Return list of pairs "type name" with "directory" or "file" as type

```
read path
```
* path - path to file
Return file value

```
delete path
```
* path - path to object that must be deleted
Delete the specified object

```
create_file path_to_file length
```
* path_to_file - path to new file
* length - length of new file
* After sending the command client must send value of new file
Create new file

```
rewrite_file file_path length
```
* path_to_file - path to file
* length - length of new file value
* After sending the command client must send new value of file
Rewrite file


```
create_or_rewrite_file path_to_file length
```
* path_to_file - path to file
* length - length of file value
* After sending the command client must send value of file
  Rewrite file or create it if it doesn't exist

```
change_data old_path --name new_name --dir new_directory
```

* old_path - path to file
* new_name - new file name
* new_directory - new file parent directory
  Change file data

```
change_file_data path --name new_name --dir new_directory
```
* path - path to file
* new_name - new name of file
* new_directory - parent directory path
  Change file or directory data

```
change_file_data path --name new_name --dir new_directory
```
* path - path to file
* new_name - new name of file
* new_directory - parent directory path
Change file data

```
create_directory path_to_directory
```
* path_to_directory - path to directory that must be created
Create new file

```
change_directory_name path --name new_name --dir new_directory 
```
* path - path to directory
* new_name - new name of directory
* new_directory - parent directory path
Change directory data

```
replace_file old_path new_path
```
Outdated command, use change_data instead

* old_path - path to file that must be replaced
* new_path - new file place
  Replace file
