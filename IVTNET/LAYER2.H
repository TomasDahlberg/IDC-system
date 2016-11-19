struct _header
{
  unsigned char size,
                version,
                targetMaster, targetNode,
                sourceMaster, sourceNode,
                command;
};

struct _packet {
  struct _header header;
  unsigned char data[251];
};

#define MAX_BUF_SIZE 256

union _mix
{
  char buffer[MAX_BUF_SIZE];
  struct {
    struct _header head;
    char buf[MAX_BUF_SIZE - sizeof(struct _header)];
  } frame;
};


struct _reply 
{
  struct _header head;
  unsigned char lchs;
};             

struct _reply *mkReplyBlock();
int allBlockChs();
int headerChs();
int poll_node();
char *allocateRouteBuffer();
char *routeBufPtr();
int freeRouteBuffer();

