n = int(input())

number = 0
next_number = 1

while n > 0
	new_next_number = number + next_number
	number = next_number
	next_number = new_next_number
	n = n - 1
	
print(number)
