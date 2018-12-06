module ADC2BRAM(clk,addr,data,BRAM_EN,BRAM_WE,Capture_trig,
S00_axis_tready,
S00_axis_tdata,
S00_axis_tvalid,
S01_axis_tready,
S01_axis_tdata,
S01_axis_tvalid);

input clk;
input Capture_trig;
input [15:0] S00_axis_tdata;
input S00_axis_tvalid;
input [15:0] S01_axis_tdata;
input S01_axis_tvalid;

output [31:0] addr;
output [31:0] data;
output BRAM_EN;
output BRAM_WE;
output S00_axis_tready;
output S01_axis_tready;

reg [31:0] addr = 32'd0;
wire  S00_axis_tready;
wire  S01_axis_tready;
reg   BRAM_WE;
wire [31:0] data;
wire BRAM_EN;

parameter  MAXADDR = 32'd65536;

assign data = {S00_axis_tdata, S01_axis_tdata};
assign BRAM_EN=1'b1;
assign S00_axis_tready = 1'b1;
assign S01_axis_tready = 1'b1;

reg Capture_trig_r1,Capture_trig_r2,Capture_trig_r3;
always @(posedge clk)
begin
	Capture_trig_r1	<= Capture_trig;
	Capture_trig_r2	<= Capture_trig_r1;
	Capture_trig_r3	<= Capture_trig_r2;
end
always @(posedge clk)
begin
	if(!Capture_trig_r3 && Capture_trig_r2)
	begin
		addr	<= 32'd0;
		BRAM_WE	<= 1'b1;
	end
	else
	begin
		if(S00_axis_tvalid && S01_axis_tvalid)
		begin
			if(addr==MAXADDR)
			begin
				addr	<= addr;
				BRAM_WE	<= 1'b0;
			end
			else
			begin
				addr	<= addr + 4;
				BRAM_WE	<= BRAM_WE;
			end	
		end
		else
		begin
			addr	<= 32'd0;
			BRAM_WE	<= 1'b0;
		end
	end
end

endmodule
