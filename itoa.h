#ifndef __ITOA_H__
#define __ITOA_H__

char * itoa(int n)
{
	char * s = new char[2*sizeof(int)+1];
	for (int i=2*sizeof(int) - 1; i>=0; i--)
	{
		s[7-i] = "0123456789ABCDEF"[((n >> i*4) & 0xF)];
	}
	s[8] = '\0';
	return s;
}

#endif
