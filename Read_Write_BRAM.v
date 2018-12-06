module R_W_BRAM(clk,R_addr,R_data_OUT,R_BRAM_EN,R_BRAM_WE,Capture_trig,
W_addr,
W_data_IN,
W_BRAM_EN,
W_BRAM_WE
);

input clk;
input Capture_trig;

output [31:0] R_addr;
output [31:0] W_addr;
input [31:0] R_data_OUT;
output [31:0] W_data_IN;
output R_BRAM_EN;
output R_BRAM_WE;
output W_BRAM_EN;
output W_BRAM_WE;

reg [31:0] R_addr = 32'd0;
reg [31:0] W_addr = 32'd0;
reg   W_BRAM_WE;

wire [31:0] R_addr;
wire [31:0] W_addr;
wire [31:0] R_data_OUT;
wire [31:0] W_data_IN;
wire   W_BRAM_WE;

parameter  MAXADDR = 32'd8192;

assign W_data_IN = R_data_OUT;
assign R_BRAM_EN=1'b1;
assign W_BRAM_EN=1'b1;
assign R_BRAM_WE=1'b0;

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
		W_addr	<= 32'd0;
		R_addr	<= 32'd0;
		W_BRAM_WE	<= 1'b0;

	end
	else
	begin
		if(addr==MAXADDR)
		begin
			R_addr	<= R_addr;
			W_addr	<= W_addr;
			W_BRAM_WE	<= 1'b0;
		end
		else
		begin
			addr	<= addr + 4;
			W_BRAM_WE	<= W_BRAM_WE;
		end	
	end
end

endmodule
