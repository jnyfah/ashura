#pragma once

#include "ashura/std/types.h"

namespace ash
{

constexpr f32 GAUSSIAN_WEIGHTS_RADIUS_2[] = {
    0.38883081312055, 0.43276926113573877, 0.17839992574371122};

constexpr f32 GAUSSIAN_WEIGHTS_RADIUS_4[] = {
    0.15642123799829394, 0.26718801880015064, 0.29738065394682034,
    0.21568339342709997, 0.06332669582763516};

constexpr f32 GAUSSIAN_WEIGHTS_RADIUS_8[] = {
    0.012886119174695622, 0.0519163052253057,   0.1361482870984158,
    0.23255915602238483,  0.2588386792559968,   0.18772977983330918,
    0.08870474727392855,  0.027292496709325292, 0.003924429406638234};

constexpr f32 GAUSSIAN_WEIGHTS_RADIUS_16[] = {
    6.531899156556559e-7,     0.000014791298968627152, 0.00021720986764341157,
    0.0020706559053401204,    0.012826757713634169,    0.05167714650813829,
    0.13552110360479683,      0.23148784424126953,     0.25764630768379954,
    0.18686497997661272,      0.0882961181645837,      0.027166770533840135,
    0.0054386298156352516,    0.0007078187356988374,   0.00005983099317322662,
    0.0000032814299066650715, 1.0033704349693544e-7};

}        // namespace ash
