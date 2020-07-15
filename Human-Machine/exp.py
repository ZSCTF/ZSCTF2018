from pwn import *
#p = remote('127.0.0.1', 30001)
p=process('./game_2018')
flag = [0 for i in range(1024)]
global times
times =0
def check(l,r):
    global times
    times = times+1
    p.send('? %d %d\n'%(l,r))
    ans = p.recvuntil('\n')
    return int(ans)
def dfs(l,r,n):
    global times

    if l+1==r:
        flag[l]=n
        return
    try:
        mid = (l + r) / 2
        t = check(l,mid)

    except:
        print "times",times
    else:
        if (t > 0):
            dfs(l,mid, t)
        if (n-t > 0):
            dfs(mid,r, n-t)


time=[1,2,4,6,8,10,12,14]
for k in time:
	ans = '!'
	print p.recvuntil('n=%d\n'%k)
	dfs(0,1024,k)
	for i in range(0,1024):
		if flag[i]>0:
			ans = ans + ' ' + str(i)
	print ans
	p.send(ans+"\n")
	flag = [0 for i in range(1024)]
	
print p.recv(1024)








'''
ans = '!'
dfs(0,1024,2)
for i in range(0,1024):
    if flag[i]>0:
        ans = ans + ' ' + str(i)
print ans
p.send(ans+"\n")
print p.recvuntil('n=4\n')
flag = [0 for i in range(1024)]
ans = '!'
dfs(0,1024,4)
for i in range(0,1024):
    if flag[i]>0:
        ans = ans + ' ' + str(i)
print ans
p.send(ans+"\n")
print p.recvuntil('n=6\n')
flag = [0 for i in range(1024)]
ans = '!'
dfs(0,1024,6)
for i in range(0,1024):
    if flag[i]>0:
        ans = ans + ' ' + str(i)
print ans
p.send(ans+"\n")
print p.recvuntil('n=8\n')
times =0
flag = [0 for i in range(1024)]
ans = '!'
dfs(0,1024,8)
for i in range(0,1024):
    if flag[i]>0:
        ans = ans + ' ' + str(i)
print ans
p.send(ans+"\n")
print p.recvuntil('n=10\n')
times =0
flag = [0 for i in range(1024)]
ans = '!'
dfs(0,1024,10)
for i in range(0,1024):
    if flag[i]>0:
        for j in range(0,flag[i]):
            ans = ans + ' ' + str(i)
print ans
p.send(ans+"\n")


print p.recvuntil('n=12\n')
times =0
flag = [0 for i in range(1024)]
ans = '!'
dfs(0,1024,12)
for i in range(0,1024):
    if flag[i]>0:
        for j in range(0,flag[i]):
            ans = ans + ' ' + str(i)
print ans
p.send(ans+"\n")



print p.recvuntil('n=14\n')
times =0
flag = [0 for i in range(1024)]
ans = '!'
dfs(0,1024,14)
for i in range(0,1024):
    if flag[i]>0:
        for j in range(0,flag[i]):
            ans = ans + ' ' + str(i)
print ans
p.send(ans+"\n")
'''
