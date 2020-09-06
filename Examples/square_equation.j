def diskr(a, b, c)
	return sqr(b) - 4 * a * c


a = input()
b = input()
c = input()

D = diskr(a, b, c)

if D > 0
	print((-b + sqrt(D)) / (2*a))
	print((-b - sqrt(D)) / (2*a))
	
if D == 0
	print((-b) / (2 * a))	
