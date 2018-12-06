module MTS_CLK(
    PLsysref_in_diff_n,
    PLsysref_in_diff_p,
	FPGA_clk_n,
    FPGA_clk_p,
    ADC_pl_clk,
    DAC_pl_clk,
    
	user_sysref_adc,
	user_sysref_dac,
	FPGAPL_ref_clk
);
	input PLsysref_in_diff_n;
	input PLsysref_in_diff_p;
	input FPGA_clk_n;
	input FPGA_clk_p;
	input ADC_pl_clk;
	input DAC_pl_clk;
	
    output user_sysref_adc;
    output user_sysref_dac;
    output FPGAPL_ref_clk;
	
	wire PLsysref_in_diff_n;
	wire PLsysref_in_diff_p;
	wire FPGA_clk_n;
	wire FPGA_clk_p;
    wire ADC_pl_clk;
    wire DAC_pl_clk;
        
	reg user_sysref_adc;
	reg user_sysref_dac;
	wire FPGAPL_ref_clk;
	
        wire PLsysref_i;
        reg PL_sysref_sync;

diff_single_g PLsysref_inst(
	.clk_in_p(PLsysref_in_diff_p),
	.clk_in_n(PLsysref_in_diff_n),
	.clk(PLsysref_i));

diff_single_g FPGAPL_ref_clk_inst(
	.clk_in_p(FPGA_clk_p),
	.clk_in_n(FPGA_clk_n),
	.clk(FPGAPL_ref_clk));

always @(posedge FPGAPL_ref_clk)
begin
	PL_sysref_sync	<= PLsysref_i;
end

always @(posedge ADC_pl_clk)
begin
	user_sysref_adc	<= PL_sysref_sync;
end

always @(posedge DAC_pl_clk)
begin
	user_sysref_dac	<= PL_sysref_sync;
end
endmodule
