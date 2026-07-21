function [Data,LastReadIndex]=MySharedDLL_getDataBlock(start_index,copy_count)
PacketWidth=calllib('MySharedDLL_A', 'getPacketWidth');
SrcBufDim=calllib('MySharedDLL_A', 'getNumOfPacketsInBuffer'); % 300 items in a circular buffer; indexstarting at 0

if start_index+copy_count>SrcBufDim, % we must perform copying in two continous source blocks:
   LastReadIndex=start_index+copy_count-SrcBufDim-1;
   copy_ranges=[start_index,SrcBufDim-1 ...
               ;          0,LastReadIndex];
else                               %** we can copy in a single block:
   LastReadIndex=start_index+copy_count-1;
   copy_ranges=[start_index,LastReadIndex];
end;

Data=zeros(copy_count,PacketWidth);
v=zeros(1,copy_count*PacketWidth);
N=0;
for i=1:size(copy_ranges,1),
   pv = libpointer('doublePtr', v);
   [pv1,v1]=calllib('MySharedDLL_A', 'getIndexedDataBlock',pv,copy_ranges(i,1),copy_ranges(i,2));
   N1=diff(copy_ranges(i,:))+1;
   Data(N+1:N+N1,:)=reshape(v1(1:N1*PacketWidth),PacketWidth,N1)';
   N=N+N1;
end;
end