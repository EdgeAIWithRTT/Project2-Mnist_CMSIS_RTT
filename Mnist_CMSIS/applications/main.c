/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-07-15     lebhoryi     V0.0.1
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <stdint.h>
#include <stdio.h>
#include <arm_math.h>
#include <arm_nnfunctions.h>
#include "mnist_parameters.h"
#include "inputdata.h"

/* conv1 weights */
const  q7_t conv1_wt[INPUT] = CONV1_WT;
const q7_t conv1_bias[CONV1_OUT_CH] = CONV1_BIAS;

/* conv2 weights */
const  q7_t conv2_wt[INPUT2] = CONV2_WT;
const  q7_t conv2_bias[CONV2_OUT_CH] = CONV2_BIAS;

/* fully connected weights */
const  q7_t fc1_wt[FC1_DIM * FC1_OUT] = FC1_WT;
const  q7_t fc1_bias[FC1_OUT] = FC1_BIAS;

// /* network function */
static uint8_t net(q7_t*, q7_t*);


// image buffer
q7_t img_buffer2[4*28*28];
q7_t data[784] = IMG5_1;

int main(void)
{
    uint8_t index = 0;
    
    // output data
    q7_t output_data[FC1_OUT] = {0};

    index = net(data, output_data);
    rt_kprintf("y_true: %d \n", 1);
    rt_kprintf("y_pred: %d \n", index);
    rt_kprintf("==============================================================\n");
    // rt_kprintf("score : %d\% \n", output_data[index]);
    return RT_EOK;
}

// network
static uint8_t net(q7_t image_data[], q7_t output_data[])
{
    // Conv1
    arm_convolve_HWC_q7_basic (image_data, CONV1_IM_DIM, CONV1_IM_CH, conv1_wt,
            CONV1_OUT_CH, CONV1_KERNEL_DIM, CONV1_PADDING, CONV1_STRIDE, conv1_bias,
            CONV1_BIAS_LSHIFT, CONV1_OUT_RSHIFT, img_buffer2, CONV1_OUT_DIM, NULL,
            NULL);
    arm_relu_q7 (img_buffer2, CONV1_OUT_DIM * CONV1_OUT_DIM * CONV1_OUT_CH);
    arm_maxpool_q7_HWC (img_buffer2, CONV1_OUT_DIM, CONV1_OUT_CH, POOL1_KERNEL_DIM,
            POOL1_PADDING, POOL1_STRIDE, POOL1_OUT_DIM, NULL, image_data);

    // Conv2
    arm_convolve_HWC_q7_basic (image_data, CONV2_IM_DIM, CONV2_IM_CH, conv2_wt,
            CONV2_OUT_CH, CONV2_KERNEL_DIM, CONV2_PADDING, CONV2_STRIDE, conv2_bias,
            CONV2_BIAS_LSHIFT, CONV2_OUT_RSHIFT, img_buffer2, CONV2_OUT_DIM, NULL,
            NULL);
    arm_relu_q7 (img_buffer2, CONV2_OUT_DIM * CONV2_OUT_DIM * CONV2_OUT_CH);
    arm_maxpool_q7_HWC (img_buffer2, CONV2_OUT_DIM, CONV2_OUT_CH, POOL2_KERNEL_DIM,
            POOL2_PADDING, POOL2_STRIDE, POOL2_OUT_DIM, NULL, image_data);

    // FC
    arm_fully_connected_q7 (image_data, fc1_wt, FC1_DIM, FC1_OUT, FC1_BIAS_LSHIFT,
            FC1_OUT_RSHIFT, fc1_bias, output_data, NULL);

    arm_softmax_q7 (output_data, FC1_OUT, output_data);
    uint8_t x = 0;
    for (int i = 0; i < 10; i++)
    {
        // rt_kprintf(output_data[i]);
        if (output_data[i] > output_data[x])
            x = i;
    }
    return x;
}
