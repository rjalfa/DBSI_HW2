# DBSI_HW2
DBSI HW 2

This assignment required us to experiment with different kinds of indices and calculate cost of executing a query. The code is written in C++ and the WHERE condition Bitmaps have been created partly with Python.

# Organization for Usage
There are four main executables in the assignment. They are presented in the order they should be executed as the output for a earlier version is a dependancy for the next:
1. *disk* : This creates an empty disk of specified number of blocks
2. *generatetable* : This creates a table of sales with specified number of records on the disk created earlier.
3. *createindex* : This creates a bitmap [both RowID and bit-array] index and a bitsliced index on the amount field of the table created in the earlier step.
4. *runquery* : This runs the query specified in the assignment, accepting the WHERE clause vector from STDIN

# Classes
The code is written using OOP concepts. The main classes are as follows:
1. *Disk* : Disk handler class
2. *Block* : Abstract Class for storing a disk block. Has Concrete Classes : RecordBlock, BitmapBlock and RowIDBlock
3. *Record*: a structure that stores data for a record
4. *Index* : Abstract Class representing index, Has Concrete Class Bitslice and abstraction class Bitmap
5. *Bitmap* : Abstract Class representing bitmap indices. Has Concrete Class Bitarray and RowIdBitmap

All the above classes have suitable constructors, destructors and manager functions.

