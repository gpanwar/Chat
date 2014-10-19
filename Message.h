struct MESSAGE {
	int msg_type;		//0 - Username auth //1 - Message //2 - Menu	//3 - Auth user for chat
	char payload[512];
};

