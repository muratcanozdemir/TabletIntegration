loadlibrary MySharedDLL MySharedDLL.h  alias MySharedDLL_A
x=libisloaded('MySharedDLL_A');
fprintf('libisloaded(''MySharedDLL'')=%d\n',x);
libfunctions MySharedDLL_A -full

%** test original read write functions: *******
str1='ABCD';
calllib('MySharedDLL_A', 'SetSharedMem', str1);
strbuf=repmat('A',1,20);
c_str2 = libpointer('cstring', strbuf);
c_str2 = calllib('MySharedDLL_A', 'GetSharedMem', c_str2,length(str1)+1);
fprintf('str1=%s, str2=%s\n\n',str1,c_str2);
%******************  read write indices *****************************
y=20;
calllib('MySharedDLL_A', 'setReadIndex', y);
x=calllib('MySharedDLL_A', 'getReadIndex');
fprintf('%d written to ReadIndex; %d read from ReadIndex\n',y,x);

y=7476;
calllib('MySharedDLL_A', 'setWriteIndex', y);
x=calllib('MySharedDLL_A', 'getWriteIndex');
fprintf('%d written to writeIndex; %d read from WriteIndex\n',y,x);

x=calllib('MySharedDLL_A', 'getReadIndex');
fprintf('%d read from ReadIndex\n\n',x);

%************* read write data packets : ******************************
PacketWidth=calllib('MySharedDLL_A', 'getPacketWidth');
NumOfPacketsInBuffer=calllib('MySharedDLL_A', 'getNumOfPacketsInBuffer');
PacketBSize=calllib('MySharedDLL_A', 'getPacketBSize');
fprintf('PacketBSize=%d\n',PacketBSize);
windex=NumOfPacketsInBuffer-1;
v1 = rand(1,PacketWidth);   % Attention: length of this vector MUST equal PacketWidth !!!!
pv1 = libpointer('doublePtr', v1);
calllib('MySharedDLL_A', 'setIndexedDataPacket',pv1,windex);
calllib('MySharedDLL_A', 'setWriteIndex', windex);

v2 = repmat(999,1,PacketWidth);   % Attention: length of this vector MUST equal PacketWidth !!!!
pv2 = libpointer('doublePtr', v2);
[pv3,v3]=calllib('MySharedDLL_A', 'getIndexedDataPacket',pv2,windex);
x=calllib('MySharedDLL_A', 'getWriteIndex');
fprintf('%d read from WriteIndex\n\n',x);
fprintf('Test getIndexedDataPacket:\n');
fprintf('maximum absolute difference between written and read data packet: %10.6f\n',max(abs(v1-v3(1:PacketWidth))));

[pv3,v4]=calllib('MySharedDLL_A', 'getLastWrittenDataPacket',pv2);
fprintf('Test getLastWrittenDataPacket:\n');
fprintf('maximum absolute difference between written and read data packet: %10.6f\n\n',max(abs(v1-v4(1:PacketWidth))));

%************* cyclic writing and reading : ******************************

calllib('MySharedDLL_A', 'setReadIndex', 99);
calllib('MySharedDLL_A', 'setWriteIndex',100);

N=NumOfPacketsInBuffer-2;
for i=1:N,
   v1=randn(1,PacketWidth);
   pv1 = libpointer('doublePtr', v1);
   err=calllib('MySharedDLL_A','setNextDataPacket',pv1);
   [pv3,v3]=calllib('MySharedDLL_A', 'getLastWrittenDataPacket',pv2);
   if err || max(abs(v1-v3))>1e-10,
      break;
   end;
end;

x=calllib('MySharedDLL_A', 'getWriteIndex');
fprintf('%d read from WriteIndex\n\n',x);


%***** test Data Blockreading: ****
N=NumOfPacketsInBuffer;
i_start=200;
Data1=randn(N,PacketWidth)*100;
for i=1:N,
   v1=Data1(i,:);
   pv1 = libpointer('doublePtr', v1);
   iw=mod(i_start+i-1,NumOfPacketsInBuffer);
   calllib('MySharedDLL_A', 'setIndexedDataPacket',pv1,iw);
end;

Data2=MySharedDLL_getDataBlock(i_start,N);
fprintf('Block reading test: max error=%10.5f\n',max(max(abs(Data2-Data1))));
unloadlibrary MySharedDLL_A
x=libisloaded('MySharedDLL_A');
fprintf('libisloaded(''MySharedDLL'')=%d\n',x);
