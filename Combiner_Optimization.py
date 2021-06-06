##Experimental code for combination optimization of 2 product with 2 limitation parameters
##code by Reefinza
o = float(input("min value for var 1:"))
p = float(input("max value for var 1:"))
q = float(input("min value for var 2:"))
r = float(input("max value for var 2:"))
f = open("result.txt", "w")
f.write("var1-pro1: ")
f.write("   ")
f.write("var2-pro1: ")
f.write("   ")
f.write("var1-pro2: ")
f.write("   ")
f.write("var2-pro2: ")
f.write("   ")
f.write("number of pro1: ")
f.write("   ")
f.write("number of pro2: ")
f.write("   ")
f.write("Total: ")
f.write("   ")
f.write("var1: ")
f.write("   ")
f.write("var2: ")
f.write("\n")


while True:
    x = float(input("var 1 for product 1:"))
    s = float(input("var 2 for product 1:"))
    a = float(input("number of product 1:"))
    y = float(input("var 1 for product 2:"))
    t = float(input("var 2 for product 2:"))
    b = float(input("number of product 2:"))
    
    i=1

    while i<=a:
        c=1
        current_max=0
        imax=0
        cmax=0
        while c<=b:
            m=(i*x+c*y)/(i+c)
            n=(i*s+c*t)/(i+c)
            if m<=p and m>=o and n<=r and n>=q:
                ic=i+c
                current_max=max(current_max,ic)
                imax=max(imax,i)
                cmax=max(cmax,c)
            c+=1
        i+=1
    if(imax>0 and cmax>0):
        print(imax,cmax,current_max,m,n)
        f = open("result.txt", "a")
        f.write(str(x))
        f.write("   ")
        f.write(str(s))
        f.write("   ")
        f.write(str(y))
        f.write("   ")
        f.write(str(t))
        f.write("   ")
        f.write(str(imax))
        f.write("   ")
        f.write(str(cmax))
        f.write("   ")
        f.write(str(current_max))
        f.write("   ")
        f.write(str(m))
        f.write("   ")
        f.write(str(n))
        f.write("\n")
        f.close()
    else:
        print("combination not found")
        f = open("result.txt", "a")
        f.write(str(x))
        f.write("   ")
        f.write(str(s))
        f.write("   ")
        f.write(str(y))
        f.write("   ")
        f.write(str(t))
        f.write("   ")
        f.write("combination not found")
        f.write("\n")
        f.close()
    
    if input('Do You Want To Continue?(y/n) ') != 'y':
        break