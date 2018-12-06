module Loopback_IQ(m00_axis_tdata,m00_axis_tready,m00_axis_tvalid,
S00_axis_tready,
S00_axis_tdata,
S00_axis_tvalid,
S01_axis_tready,
S01_axis_tdata,
S01_axis_tvalid);

input [15:0] S00_axis_tdata;
input S00_axis_tvalid;
input [15:0] S01_axis_tdata;
input S01_axis_tvalid;

input m00_axis_tready;
output [31:0] m00_axis_tdata;
output m00_axis_tvalid;

output S00_axis_tready;
output S01_axis_tready;

wire  S00_axis_tready;
wire  S01_axis_tready;
wire [31:0] m00_axis_tdata;
wire m00_axis_tvalid;

assign m00_axis_tdata = {S01_axis_tdata, S00_axis_tdata};
assign m00_axis_tvalid = S00_axis_tvalid;
assign S00_axis_tready = m00_axis_tready;
assign S01_axis_tready = m00_axis_tready;

endmodule
