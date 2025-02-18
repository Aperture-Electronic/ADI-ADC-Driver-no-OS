/***************************************************************************//**
 *   @file   ad_fmcdaq3_ebz.c
 *   @brief  Implementation of Main Function.
 *   @author DBogdan (dragos.bogdan@analog.com)
 *******************************************************************************
 * Copyright 2015(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "platform_drivers.h"
#include "ad9152.h"
#include "ad9528.h"
#include "ad9680.h"
#include "adc_core.h"
#include "dac_core.h"
#include "xcvr_core.h"
#include "jesd_core.h"
#include "dmac_core.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define GPIO_CLKD_STATUS_0      32
#define GPIO_CLKD_STATUS_1      33
#define GPIO_DAC_IRQ            34
#define GPIO_ADC_FDA            35
#define GPIO_ADC_FDB            36
#define GPIO_DAC_TXEN           37
#define GPIO_ADC_PD             38
#define GPIO_TRIG               39

/***************************************************************************//**
 * @brief main
 ******************************************************************************/
int main(void)
{
	struct ad9528_dev* ad9528_device;
	struct ad9152_dev* ad9152_device;
	struct ad9680_dev* ad9680_device;
	struct ad9528_channel_spec ad9528_channels[8];
	struct ad9528_init_param ad9528_param;
	struct ad9152_init_param ad9152_param;
	struct ad9680_init_param ad9680_param;
	struct ad9528_platform_data ad9528_pdata;
	spi_init_param ad9528_spi_param;
	spi_init_param ad9152_spi_param;
	spi_init_param ad9680_spi_param;
	xcvr_core ad9152_xcvr;
	jesd_core ad9152_jesd;
	dac_channel ad9152_channels[2];
	dac_core ad9152_core;
	xcvr_core ad9680_xcvr;
	jesd_core ad9680_jesd;
	adc_core ad9680_core;
	dmac_core ad9680_dma;
	dmac_xfer rx_xfer;

	// base addresses

#ifdef XILINX
	ad9152_xcvr.base_address = XPAR_AXI_AD9152_XCVR_BASEADDR;
	ad9152_core.base_address = XPAR_AXI_AD9152_CORE_BASEADDR;
	ad9680_xcvr.base_address = XPAR_AXI_AD9680_XCVR_BASEADDR;
	ad9680_core.base_address = XPAR_AXI_AD9680_CORE_BASEADDR;
	ad9152_jesd.base_address = XPAR_AXI_AD9152_JESD_TX_AXI_BASEADDR;
	ad9680_jesd.base_address = XPAR_AXI_AD9680_JESD_RX_AXI_BASEADDR;
	ad9680_dma.base_address = XPAR_AXI_AD9680_DMA_BASEADDR;
#endif
#ifdef ALTERA
	ad9152_xcvr.base_address = AD9152_JESD204_LINK_MANAGEMENT_BASE;
	ad9152_xcvr.dev.link_pll.base_address =
			AD9152_JESD204_LINK_PLL_RECONFIG_BASE;
	ad9152_xcvr.dev.atx_pll.base_address =
			AD9152_JESD204_LANE_PLL_RECONFIG_BASE;
	ad9152_core.base_address = AXI_AD9152_CORE_BASE;
	AD9680_XCVR.BASE_ADDRESS = AD9680_JESD204_LINK_MANAGEMENT_BASE;
	ad9680_xcvr.dev.link_pll.base_address =
			AD9680_JESD204_LINK_PLL_RECONFIG_BASE;
	ad9680_core.base_address = AXI_AD9680_CORE_BASE;
	ad9152_jesd.base_address = AD9152_JESD204_LINK_RECONFIG_BASE;
	ad9680_jesd.base_address = AD9680_JESD204_LINK_RECONFIG_BASE;

	ad9152_xcvr.dev.channel_pll[0].type = cmu_tx_type;
	ad9680_xcvr.dev.channel_pll[0].type = cmu_cdr_type;
	ad9152_xcvr.dev.channel_pll[0].base_address = AVL_ADXCFG_0_RCFG_S0_BASE;
	ad9680_xcvr.dev.channel_pll[0].base_address = AVL_ADXCFG_0_RCFG_S1_BASE;

	ad9680_dma.base_address = AXI_AD9680_DMA_BASE;
	ad9152_dma.base_address = AXI_AD9152_DMA_BASE;
	rx_xfer.start_address =  0x800000;
	tx_xfer.start_address =  0x900000;;
#endif

	ad9528_spi_param.type = ZYNQ_PS7_SPI;
	ad9152_spi_param.type = ZYNQ_PS7_SPI;
	ad9680_spi_param.type = ZYNQ_PS7_SPI;
	ad9528_spi_param.chip_select = 0x6;
	ad9152_spi_param.chip_select = 0x5;
	ad9680_spi_param.chip_select = 0x3;
	ad9528_spi_param.cpha = 0;
	ad9152_spi_param.cpha = 0;
	ad9680_spi_param.cpha = 0;
	ad9528_spi_param.cpol = 0;
	ad9152_spi_param.cpol = 0;
	ad9680_spi_param.cpol = 0;

	ad9528_param.spi_init = ad9528_spi_param;
	ad9152_param.spi_init = ad9152_spi_param;
	ad9680_param.spi_init = ad9680_spi_param;
	// ad9528 defaults

	ad9528_param.pdata = &ad9528_pdata;

	ad9528_param.pdata->num_channels = 8;
	ad9528_param.pdata->channels = &ad9528_channels[0];
	ad9528_init(&ad9528_param);

	// dac-device-clock (1.233G)
	ad9528_channels[0].channel_num = 2;
	ad9528_channels[0].channel_divider = 1;

	//adc sysref (4.9M)
	ad9528_channels[1].channel_num = 8;
	ad9528_channels[1].channel_divider = 256;

	// adc-fpga-clock (616.5M)
	ad9528_channels[2].channel_num = 9;
	ad9528_channels[2].channel_divider = 2;

	// adc dev sysref (4.9M)
	ad9528_channels[3].channel_num = 7;
	ad9528_channels[3].channel_divider = 256;

	// adc-device-clock (1.233G)
	ad9528_channels[4].channel_num = 13;
	ad9528_channels[4].channel_divider = 1;

	// dac sysref (4.9M)
	ad9528_channels[5].channel_num = 5;
	ad9528_channels[5].channel_divider = 256;

	// dac-fpga-fmc (616.5M)
	ad9528_channels[6].channel_num = 4;
	ad9528_channels[6].channel_divider = 2;

	// dac dev sysref (4.9M)
	ad9528_channels[7].channel_num = 6;
	ad9528_channels[7].channel_divider = 256;

	// pllx settings

	ad9528_param.pdata->spi3wire = 1;
	ad9528_param.pdata->vcxo_freq = 100000000;
	ad9528_param.pdata->osc_in_diff_en = 1;
	ad9528_param.pdata->pll2_charge_pump_current_n_a = 35000;
	ad9528_param.pdata->pll2_vco_diff_m1 = 3;
	ad9528_param.pdata->pll2_r1_div = 3;
	ad9528_param.pdata->pll2_ndiv_a_cnt = 3;
	ad9528_param.pdata->pll2_ndiv_b_cnt = 27;
	ad9528_param.pdata->pll2_n2_div = 37;
	ad9528_param.pdata->sysref_k_div = 128;
	ad9528_param.pdata->rpole2 = RPOLE2_900_OHM;
	ad9528_param.pdata->rzero = RZERO_1850_OHM;
	ad9528_param.pdata->cpole1 = CPOLE1_16_PF;

	// dac settings
	xcvr_getconfig(&ad9152_xcvr);
	ad9152_xcvr.reconfig_bypass = 0;
	ad9152_xcvr.ref_clock_khz = 616500;
	ad9152_xcvr.lane_rate_kbps = 12330000;
#ifdef XILINX
	ad9152_xcvr.dev.qpll_enable = 1;
#endif

	ad9152_jesd.rx_tx_n = 0;
	ad9152_jesd.scramble_enable = 1;
	ad9152_jesd.octets_per_frame = 1;
	ad9152_jesd.frames_per_multiframe = 32;
	ad9152_jesd.subclass_mode = 1;

	ad9152_channels[0].dds_dual_tone = 0;
	ad9152_channels[0].dds_frequency_0 = 33*1000*1000;
	ad9152_channels[0].dds_phase_0 = 0;
	ad9152_channels[0].dds_scale_0 = 500000;
	ad9152_channels[0].sel = DAC_SRC_DDS;
	ad9152_channels[1].dds_dual_tone = 0;
	ad9152_channels[1].dds_frequency_0 = 11*1000*1000;
	ad9152_channels[1].dds_phase_0 = 0;
	ad9152_channels[1].dds_scale_0 = 500000;
	ad9152_channels[0].pat_data = 0xb1b0a1a0;
	ad9152_channels[1].pat_data = 0xd1d0c1c0;
	ad9152_channels[1].sel = DAC_SRC_DDS;

	ad9152_core.no_of_channels = 2;
	ad9152_core.resolution = 16;
	ad9152_core.channels = &ad9152_channels[0];

	ad9152_param.stpl_samples[0][0] =
			(ad9152_channels[0].pat_data >> 0)  & 0xffff;
	ad9152_param.stpl_samples[0][1] =
			(ad9152_channels[0].pat_data >> 16) & 0xffff;
	ad9152_param.stpl_samples[0][2] =
			(ad9152_channels[0].pat_data >> 0)  & 0xffff;
	ad9152_param.stpl_samples[0][3] =
			(ad9152_channels[0].pat_data >> 16) & 0xffff;
	ad9152_param.stpl_samples[1][0] =
			(ad9152_channels[1].pat_data >> 0)  & 0xffff;
	ad9152_param.stpl_samples[1][1] =
			(ad9152_channels[1].pat_data >> 16) & 0xffff;
	ad9152_param.stpl_samples[1][2] =
			(ad9152_channels[1].pat_data >> 0)  & 0xffff;
	ad9152_param.stpl_samples[1][3] =
			(ad9152_channels[1].pat_data >> 16) & 0xffff;
	ad9152_param.interpolation = 1;
	ad9152_param.lane_rate_kbps = 12330000;

	// adc settings

	ad9680_param.lane_rate_kbps = 12330000;

	xcvr_getconfig(&ad9680_xcvr);
	ad9680_xcvr.reconfig_bypass = 0;
	ad9680_xcvr.ref_clock_khz = 616500;
	ad9680_xcvr.lane_rate_kbps = 12330000;
#ifdef XILINX
	ad9680_xcvr.dev.qpll_enable = 1;
#endif

	ad9680_jesd.rx_tx_n = 1;
	ad9680_jesd.scramble_enable = 1;
	ad9680_jesd.octets_per_frame = 1;
	ad9680_jesd.frames_per_multiframe = 32;
	ad9680_jesd.subclass_mode = 1;

	ad9680_core.no_of_channels = 2;
	ad9680_core.resolution = 14;

        // receiver DMA configuration

#ifdef ZYNQ
	rx_xfer.start_address = XPAR_DDR_MEM_BASEADDR + 0x800000;
#endif

#ifdef MICROBLAZE
	rx_xfer.start_address = XPAR_AXI_DDR_CNTRL_BASEADDR + 0x800000;
#endif
	ad9680_dma.type = DMAC_RX;
	ad9680_dma.transfer = &rx_xfer;
	rx_xfer.id = 0;
	rx_xfer.no_of_samples = 32768;

	// functions (do not modify below)

	gpio_desc *dac_txen;
	gpio_desc *adc_pd;

	gpio_get(&dac_txen, GPIO_DAC_TXEN);
	gpio_get(&adc_pd, GPIO_ADC_PD);

	ad_platform_init();
	gpio_set_value(dac_txen, 0x1);
	gpio_set_value(adc_pd, 0x0);

	ad9528_setup(&ad9528_device, ad9528_param);

	ad9152_setup(&ad9152_device, ad9152_param);

	jesd_setup(&ad9152_jesd);
	xcvr_setup(&ad9152_xcvr);
	axi_jesd204_tx_status_read(&ad9152_jesd);
	dac_setup(&ad9152_core);
	ad9152_status(ad9152_device);

	// ad9152-x1 do not support data path prbs (use short-tpl)

	ad9152_channels[0].sel = DAC_SRC_SED;
	ad9152_channels[1].sel = DAC_SRC_SED;
	dac_data_setup(&ad9152_core);
	ad9152_short_pattern_test(ad9152_device, ad9152_param);

	// ad9152-xN (n > 1) supports data path prbs

	ad9152_channels[0].sel = DAC_SRC_PN23;
	ad9152_channels[1].sel = DAC_SRC_PN23;
	dac_data_setup(&ad9152_core);
	ad9152_param.prbs_type = AD9152_TEST_PN7;
	ad9152_datapath_prbs_test(ad9152_device, ad9152_param);

	ad9152_channels[0].sel = DAC_SRC_PN31;
	ad9152_channels[1].sel = DAC_SRC_PN31;
	dac_data_setup(&ad9152_core);
	ad9152_param.prbs_type = AD9152_TEST_PN15;
	ad9152_datapath_prbs_test(ad9152_device, ad9152_param);

	ad9680_setup(&ad9680_device, &ad9680_param);
	jesd_setup(&ad9680_jesd);
	xcvr_setup(&ad9680_xcvr);
	axi_jesd204_tx_status_read(&ad9680_jesd);
	adc_setup(ad9680_core);
	ad9680_test(ad9680_device, AD9680_TEST_PN9);
	if(adc_pn_mon(ad9680_core, ADC_PN9) == -1) {
		ad_printf("%s ad9680 - PN9 sequence mismatch!\n", __func__);
	};
	ad9680_test(ad9680_device, AD9680_TEST_PN23);
	if(adc_pn_mon(ad9680_core, ADC_PN23A) == -1) {
		ad_printf("%s ad9680 - PN23 sequence mismatch!\n", __func__);
	};

	// default data

	ad9152_channels[0].sel = DAC_SRC_DDS;
	ad9152_channels[1].sel = DAC_SRC_DDS;
	dac_data_setup(&ad9152_core);
	ad9680_test(ad9680_device, AD9680_TEST_OFF);
	ad_printf("daq3: setup and configuration is done\n");

        // capture data with DMA

	if(!dmac_start_transaction(ad9680_dma)){
		ad_printf("daq3: RX capture done.\n");
        };

	ad9528_remove(ad9528_device);
	ad9152_remove(ad9152_device);
	ad9680_remove(ad9680_device);
	gpio_remove(dac_txen);
	gpio_remove(adc_pd);

	ad_platform_close();

	return(0);
}
