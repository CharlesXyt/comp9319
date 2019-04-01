//README
Student id:z5151559
Student name:Yitong Xiao

Use:
	For encoding:
		Usage:<delimiter> <path to temporary folder> <path to file need to be encoded> <path to the generated file>

	For searching 
		Usage:<delimiter> <path to BWT encoded file> <path to Index file> <options> <query string>

	<options>:
		-m :output the total number of matching substrings (count duplicates) to the standard output.
		-n :output for the number of unique matching records
		-a :output the sorted and unique record identifiers (no duplicates)
		-i :search term as "i j" and search for the records with their identifiers beginning with i and ending with j

Algorithm:
	For encoding:
		(1) Reading the file to the memory;
		(2) Build a structure which has 256 buckets and the store the file using the ASCII code as the Index. Each bucket has a array and store the position of each character;
		(3) Sort each bucket according by the ASCII code of character before that position, if they are equal then compare the character whose position is two less until get the order;
		(4) output the character on every position on each bucket in order as the encoded file;
		(5) output position in the delimiter's bucket and reconstruct as the position file;
	For searching:
		(1) Build the c table and occ table. c table is easy to get just read the file and build it. For occ table,I made a big index like the professor said. Every  0.1MB i made a index for 			storing their occ.If i want get the occ value for a position , just calculate from the nearest big index;
		(2) -m:Do backward search for each query string;
		(3) -i:through the position , we can get the exact position for specific delimiter and do backward search until get another delimiter;
		(4) -a and -n:do backward search for query string but don't stop until get a delimiter and store the position. through the position file, we can get the record number.

Problem:
	I just use $ as a delimiter for encoding and searching. If it is not i just change it to $ and encoding and search. So if using a delimiter which is not $, but there is $ in file, there will be 	a problem, my program will still recognize it as a delimiter. 




