import random

input_filename = "/home/sungan/dataset/graph500-26/graph500-26.e"
output_filename = "/home/sungan/dataset/graph500-26/graph500-26_shuffle.e"

output_file = open(output_filename, "w", encoding = "utf-8")  
 
lines = []
endflag = "0"

with open(input_filename, "r", encoding = "utf-8") as f:
    for line in f:
        if line == "\n":
            endflag = "\n"
        elif line == "":
            endflag = ""
        else:
            lines.append(line)
        
        if len(lines) == 1000000:
            random.shuffle(lines)
            random.shuffle(lines)

            for line in lines:
                output_file.write(line)
            lines = []
        
random.shuffle(lines)
random.shuffle(lines)

for line in lines:
    output_file.write(line)

    
if endflag != "0":
    output_file.write(endflag)

output_file.close()