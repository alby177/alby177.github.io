---
layout: default
title: "MySql configuration and simple commands"
---

# MySql configuration
### MacOs X
First of all it is important to download the MySql .dmg archive and install the software:
1. Choose "MySql Community Edition" which is the only one open source.
* Choose "MySql Community Server" which is the one that should be run for Qt access.
* Choose .dmg format file for easy installation.

Once the database server is installed, it is important to start its execution.
If no differently specified, the database will be run automatically when the machine is started. To change this, or to manually start the database server execution:
1. Go to "System preferences" menu
* In the last row you can find the "MySql" feature, click to open it.
* Start the server execution and modify configuration options.

# MySql database access
## Log into database Server
The databases can be accessed only when the user is inside the database server.

First of all move to the MySql directory, which is by default `/usr/local/mysql/bin`.
Once there, it is possible to access the localhost server as root using
```
./mysql -u root -p
```  

Here the user needs to insert root password. If the user is not the root, change it with the user name. If the user needs to access a server on a different machine, type:
```
./mysql -h <hostname> -u <username> -p
```
Once inside, this is what it is shown:
```
mysql>
```
Now command can be typed to access the database or create new ones.
> Always remember to put semicolon when the command is finished. If this is not done, the command line will wait for something more for the same command with
```
  ->
```
In this case just insert the semicolon and press enter.

To exit from the database server just write `quit` or `\q`.

## Database usage
### Databases management
To visualize which database have been created on the server it is sufficient to write:
```
show databases;
```
> In general, it is not important if the commands are inserted in lower case or in upper case

If one of the databases already created must be accessed, it is sufficient to write:
```
use <database_name>;
```
To create a new database the command is:
```
create database <database_name>;
```
To check which database is currently in use, the command is:
```
select database();
```

### Table management

Each database can be filled with tables that are used to store data. Each table is created with different column containing different type of data, among which:
* varchar(lenght) -> equivalent to string
* int -> integer number
* char(lenght) -> char variable
* date -> date data type. The date is expected in the format YYYY-MM--DD

So practically the table is created as:
```
create table <table_name> (<column1_name>(<column1_data_type>), ...);
```
To see which tables are already present in a database just write:
```
show tables;
```
And to check which data fields has been created just write:
```
describe <table_name>;
```
### Data insertion

Once the table is created it is time to insert the data inside it. The easiest way is to use the insert command, inserting one row after another as:
```
insert into <table_name> values ('value_for_column1', ....);
```
> It is important to put NULL or \N if the column field of the inserted row is left blank

There is another quicker way to do it, importing a formatted file inside the database. To do so create a `*.txt` file which is then filled with data. Each row con be separated from each other using a separator, but the easiest way is to create a row for every database row. Each row is composed by the column fields which must be separated by a tab in order to guarantee the correct row insertion.

> If 2 rows are equals, they will be inserted inside the database, no matter what.
Not only txt files can be used, also csv are allowed.

The command used for inserting the data is the following one:
```
load data infile '/path/<file_name>.txt' into table <table_name> lines terminated by '<separator>';
```
The last part `lines terminated by` can be omitted.

This could not work immediately, in fact there could be a configuration value named `secure-file-priv=NULL` which denies the import of the file. This must be changed to `secure-file-priv=""` in order to allow the file import.
* On **MacOS**, the configuration file must be created in order to tell the sql server the new variable value. So, stop the server execution and then create a file named *my.cnf* in /ect using `sudo nano /etc/my.cnf` adding the following lines:
```
[mysqld_safe]
[mysqld]
secure_file_priv=""
```
Than restart the database and the data import should work.
The data should be imported from a permitted folder, so in order to do so the file should be copied inside the installation folder, as default `/usr/local/mysql/bin`.

### Retrieve data
To retrieve data the basic command is `select` followed by the specific table of clause. To access all the rows of a table just write:
```
select * from <table_name>;
```
If the row to retrieve must be selected specifically, just write:
```
select * from <table_name> where <column> = '<value_looked_for>';    
```
This command retrieves all the rows with the exact <value_looked_for>. If a general value must be found:
```
select * from <table_name> where <column> like '<general_value>%';
```
In this way all the rows containing <general_value> will be retrieved.
