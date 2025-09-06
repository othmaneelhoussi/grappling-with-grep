This branch  handles the grep task by simply traversing down the given path and recursively checking every file.
It checks every file it encounters for a matching pattern. 
If the pattern is found in a file, we print out the path of the file followed by each line where the pattern was found.
Each line is numbered and it is important to note that we only color the first occurence of the pattern on the line.
