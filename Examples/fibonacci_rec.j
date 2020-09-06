def getFib(number)
	if number <= 2
		return 1
	else
		return getFib(number-1) + getFib(number-2)
	
	
n = int(input())
res = getFib(n)
print(res)
