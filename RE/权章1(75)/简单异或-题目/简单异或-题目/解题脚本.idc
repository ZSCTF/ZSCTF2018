auto i,c;
for(i=0,c=0;i<24;i++,c++)
	Message(Byte(0x41a04f-i)^Byte(0x41a052-c%3));
