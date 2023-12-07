// SPDX-License-Identifier: CC-PDDC
// original: https://dev.to/sgf4/foreach-macro-in-c-48ic

#pragma once

#define N_VA_ARGS_(_100,_99,_98,_97,_96,_95,_94,_93,_92,_91,_90,_89,_88,_87,_86,_85,_84,_83,_82,_81,_80,_79,_78,_77,_76,_75,_74,_73,_72,_71,_70,_69,_68,_67,_66,_65,_64,_63,_62,_61,_60,_59,_58,_57,_56,_55,_54,_53,_52,_51,_50,_49,_48,_47,_46,_45,_44,_43,_42,_41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1, N, ...) N
#define N_VA_ARGS(...) N_VA_ARGS_(__VA_ARGS__ __VA_OPT__(,) 100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)

#define FOREACH_0(FN, ...) 
#define FOREACH_1(FN, E, ...) FN(E) 
#define FOREACH_2(FN, E, ...) FN(E) FOREACH_1(FN, __VA_ARGS__)
#define FOREACH_3(FN, E, ...) FN(E) FOREACH_2(FN, __VA_ARGS__)
#define FOREACH_4(FN, E, ...) FN(E) FOREACH_3(FN, __VA_ARGS__)
#define FOREACH_5(FN, E, ...) FN(E) FOREACH_4(FN, __VA_ARGS__)
#define FOREACH_6(FN, E, ...) FN(E) FOREACH_5(FN, __VA_ARGS__)
#define FOREACH_7(FN, E, ...) FN(E) FOREACH_6(FN, __VA_ARGS__)
#define FOREACH_8(FN, E, ...) FN(E) FOREACH_7(FN, __VA_ARGS__)
#define FOREACH_9(FN, E, ...) FN(E) FOREACH_8(FN, __VA_ARGS__)
#define FOREACH_10(FN, E, ...) FN(E) FOREACH_9(FN, __VA_ARGS__)
#define FOREACH_11(FN, E, ...) FN(E) FOREACH_10(FN, __VA_ARGS__)
#define FOREACH_12(FN, E, ...) FN(E) FOREACH_11(FN, __VA_ARGS__)
#define FOREACH_13(FN, E, ...) FN(E) FOREACH_12(FN, __VA_ARGS__)
#define FOREACH_14(FN, E, ...) FN(E) FOREACH_13(FN, __VA_ARGS__)
#define FOREACH_15(FN, E, ...) FN(E) FOREACH_14(FN, __VA_ARGS__)
#define FOREACH_16(FN, E, ...) FN(E) FOREACH_15(FN, __VA_ARGS__)
#define FOREACH_17(FN, E, ...) FN(E) FOREACH_16(FN, __VA_ARGS__)
#define FOREACH_18(FN, E, ...) FN(E) FOREACH_17(FN, __VA_ARGS__)
#define FOREACH_19(FN, E, ...) FN(E) FOREACH_18(FN, __VA_ARGS__)
#define FOREACH_20(FN, E, ...) FN(E) FOREACH_19(FN, __VA_ARGS__)
#define FOREACH_21(FN, E, ...) FN(E) FOREACH_20(FN, __VA_ARGS__)
#define FOREACH_22(FN, E, ...) FN(E) FOREACH_21(FN, __VA_ARGS__)
#define FOREACH_23(FN, E, ...) FN(E) FOREACH_22(FN, __VA_ARGS__)
#define FOREACH_24(FN, E, ...) FN(E) FOREACH_23(FN, __VA_ARGS__)
#define FOREACH_25(FN, E, ...) FN(E) FOREACH_24(FN, __VA_ARGS__)
#define FOREACH_26(FN, E, ...) FN(E) FOREACH_25(FN, __VA_ARGS__)
#define FOREACH_27(FN, E, ...) FN(E) FOREACH_26(FN, __VA_ARGS__)
#define FOREACH_28(FN, E, ...) FN(E) FOREACH_27(FN, __VA_ARGS__)
#define FOREACH_29(FN, E, ...) FN(E) FOREACH_28(FN, __VA_ARGS__)
#define FOREACH_30(FN, E, ...) FN(E) FOREACH_29(FN, __VA_ARGS__)
#define FOREACH_31(FN, E, ...) FN(E) FOREACH_30(FN, __VA_ARGS__)
#define FOREACH_32(FN, E, ...) FN(E) FOREACH_31(FN, __VA_ARGS__)
#define FOREACH_33(FN, E, ...) FN(E) FOREACH_32(FN, __VA_ARGS__)
#define FOREACH_34(FN, E, ...) FN(E) FOREACH_33(FN, __VA_ARGS__)
#define FOREACH_35(FN, E, ...) FN(E) FOREACH_34(FN, __VA_ARGS__)
#define FOREACH_36(FN, E, ...) FN(E) FOREACH_35(FN, __VA_ARGS__)
#define FOREACH_37(FN, E, ...) FN(E) FOREACH_36(FN, __VA_ARGS__)
#define FOREACH_38(FN, E, ...) FN(E) FOREACH_37(FN, __VA_ARGS__)
#define FOREACH_39(FN, E, ...) FN(E) FOREACH_38(FN, __VA_ARGS__)
#define FOREACH_40(FN, E, ...) FN(E) FOREACH_39(FN, __VA_ARGS__)
#define FOREACH_41(FN, E, ...) FN(E) FOREACH_40(FN, __VA_ARGS__)
#define FOREACH_42(FN, E, ...) FN(E) FOREACH_41(FN, __VA_ARGS__)
#define FOREACH_43(FN, E, ...) FN(E) FOREACH_42(FN, __VA_ARGS__)
#define FOREACH_44(FN, E, ...) FN(E) FOREACH_43(FN, __VA_ARGS__)
#define FOREACH_45(FN, E, ...) FN(E) FOREACH_44(FN, __VA_ARGS__)
#define FOREACH_46(FN, E, ...) FN(E) FOREACH_45(FN, __VA_ARGS__)
#define FOREACH_47(FN, E, ...) FN(E) FOREACH_46(FN, __VA_ARGS__)
#define FOREACH_48(FN, E, ...) FN(E) FOREACH_47(FN, __VA_ARGS__)
#define FOREACH_49(FN, E, ...) FN(E) FOREACH_48(FN, __VA_ARGS__)
#define FOREACH_50(FN, E, ...) FN(E) FOREACH_49(FN, __VA_ARGS__)
#define FOREACH_51(FN, E, ...) FN(E) FOREACH_50(FN, __VA_ARGS__)
#define FOREACH_52(FN, E, ...) FN(E) FOREACH_51(FN, __VA_ARGS__)
#define FOREACH_53(FN, E, ...) FN(E) FOREACH_52(FN, __VA_ARGS__)
#define FOREACH_54(FN, E, ...) FN(E) FOREACH_53(FN, __VA_ARGS__)
#define FOREACH_55(FN, E, ...) FN(E) FOREACH_54(FN, __VA_ARGS__)
#define FOREACH_56(FN, E, ...) FN(E) FOREACH_55(FN, __VA_ARGS__)
#define FOREACH_57(FN, E, ...) FN(E) FOREACH_56(FN, __VA_ARGS__)
#define FOREACH_58(FN, E, ...) FN(E) FOREACH_57(FN, __VA_ARGS__)
#define FOREACH_59(FN, E, ...) FN(E) FOREACH_58(FN, __VA_ARGS__)
#define FOREACH_60(FN, E, ...) FN(E) FOREACH_59(FN, __VA_ARGS__)
#define FOREACH_61(FN, E, ...) FN(E) FOREACH_60(FN, __VA_ARGS__)
#define FOREACH_62(FN, E, ...) FN(E) FOREACH_61(FN, __VA_ARGS__)
#define FOREACH_63(FN, E, ...) FN(E) FOREACH_62(FN, __VA_ARGS__)
#define FOREACH_64(FN, E, ...) FN(E) FOREACH_63(FN, __VA_ARGS__)
#define FOREACH_65(FN, E, ...) FN(E) FOREACH_64(FN, __VA_ARGS__)
#define FOREACH_66(FN, E, ...) FN(E) FOREACH_65(FN, __VA_ARGS__)
#define FOREACH_67(FN, E, ...) FN(E) FOREACH_66(FN, __VA_ARGS__)
#define FOREACH_68(FN, E, ...) FN(E) FOREACH_67(FN, __VA_ARGS__)
#define FOREACH_69(FN, E, ...) FN(E) FOREACH_68(FN, __VA_ARGS__)
#define FOREACH_70(FN, E, ...) FN(E) FOREACH_69(FN, __VA_ARGS__)
#define FOREACH_71(FN, E, ...) FN(E) FOREACH_70(FN, __VA_ARGS__)
#define FOREACH_72(FN, E, ...) FN(E) FOREACH_71(FN, __VA_ARGS__)
#define FOREACH_73(FN, E, ...) FN(E) FOREACH_72(FN, __VA_ARGS__)
#define FOREACH_74(FN, E, ...) FN(E) FOREACH_73(FN, __VA_ARGS__)
#define FOREACH_75(FN, E, ...) FN(E) FOREACH_74(FN, __VA_ARGS__)
#define FOREACH_76(FN, E, ...) FN(E) FOREACH_75(FN, __VA_ARGS__)
#define FOREACH_77(FN, E, ...) FN(E) FOREACH_76(FN, __VA_ARGS__)
#define FOREACH_78(FN, E, ...) FN(E) FOREACH_77(FN, __VA_ARGS__)
#define FOREACH_79(FN, E, ...) FN(E) FOREACH_78(FN, __VA_ARGS__)
#define FOREACH_80(FN, E, ...) FN(E) FOREACH_79(FN, __VA_ARGS__)
#define FOREACH_81(FN, E, ...) FN(E) FOREACH_80(FN, __VA_ARGS__)
#define FOREACH_82(FN, E, ...) FN(E) FOREACH_81(FN, __VA_ARGS__)
#define FOREACH_83(FN, E, ...) FN(E) FOREACH_82(FN, __VA_ARGS__)
#define FOREACH_84(FN, E, ...) FN(E) FOREACH_83(FN, __VA_ARGS__)
#define FOREACH_85(FN, E, ...) FN(E) FOREACH_84(FN, __VA_ARGS__)
#define FOREACH_86(FN, E, ...) FN(E) FOREACH_85(FN, __VA_ARGS__)
#define FOREACH_87(FN, E, ...) FN(E) FOREACH_86(FN, __VA_ARGS__)
#define FOREACH_88(FN, E, ...) FN(E) FOREACH_87(FN, __VA_ARGS__)
#define FOREACH_89(FN, E, ...) FN(E) FOREACH_88(FN, __VA_ARGS__)
#define FOREACH_90(FN, E, ...) FN(E) FOREACH_89(FN, __VA_ARGS__)
#define FOREACH_91(FN, E, ...) FN(E) FOREACH_90(FN, __VA_ARGS__)
#define FOREACH_92(FN, E, ...) FN(E) FOREACH_91(FN, __VA_ARGS__)
#define FOREACH_93(FN, E, ...) FN(E) FOREACH_92(FN, __VA_ARGS__)
#define FOREACH_94(FN, E, ...) FN(E) FOREACH_93(FN, __VA_ARGS__)
#define FOREACH_95(FN, E, ...) FN(E) FOREACH_94(FN, __VA_ARGS__)
#define FOREACH_96(FN, E, ...) FN(E) FOREACH_95(FN, __VA_ARGS__)
#define FOREACH_97(FN, E, ...) FN(E) FOREACH_96(FN, __VA_ARGS__)
#define FOREACH_98(FN, E, ...) FN(E) FOREACH_97(FN, __VA_ARGS__)
#define FOREACH_99(FN, E, ...) FN(E) FOREACH_98(FN, __VA_ARGS__)
#define FOREACH_100(FN, E, ...) FN(E) FOREACH_99(FN, __VA_ARGS__)

#define FOREACH__(FN, NARGS, ...) FOREACH_##NARGS(FN, __VA_ARGS__) 
#define FOREACH_(FN, NARGS, ...) FOREACH__(FN, NARGS, __VA_ARGS__)
#define FOREACH(FN, ...) FOREACH_(FN, N_VA_ARGS(__VA_ARGS__), __VA_ARGS__)

#define FOREACHA_0(FN, ARG, ...) 
#define FOREACHA_1(FN, ARG, E, ...) FN(ARG, E) 
#define FOREACHA_2(FN, ARG, E, ...) FN(ARG, E) FOREACHA_1(FN, ARG, __VA_ARGS__)
#define FOREACHA_3(FN, ARG, E, ...) FN(ARG, E) FOREACHA_2(FN, ARG, __VA_ARGS__)
#define FOREACHA_4(FN, ARG, E, ...) FN(ARG, E) FOREACHA_3(FN, ARG, __VA_ARGS__)
#define FOREACHA_5(FN, ARG, E, ...) FN(ARG, E) FOREACHA_4(FN, ARG, __VA_ARGS__)
#define FOREACHA_6(FN, ARG, E, ...) FN(ARG, E) FOREACHA_5(FN, ARG, __VA_ARGS__)
#define FOREACHA_7(FN, ARG, E, ...) FN(ARG, E) FOREACHA_6(FN, ARG, __VA_ARGS__)
#define FOREACHA_8(FN, ARG, E, ...) FN(ARG, E) FOREACHA_7(FN, ARG, __VA_ARGS__)
#define FOREACHA_9(FN, ARG, E, ...) FN(ARG, E) FOREACHA_8(FN, ARG, __VA_ARGS__)
#define FOREACHA_10(FN, ARG, E, ...) FN(ARG, E) FOREACHA_9(FN, ARG, __VA_ARGS__)
#define FOREACHA_11(FN, ARG, E, ...) FN(ARG, E) FOREACHA_10(FN, ARG, __VA_ARGS__)
#define FOREACHA_12(FN, ARG, E, ...) FN(ARG, E) FOREACHA_11(FN, ARG, __VA_ARGS__)
#define FOREACHA_13(FN, ARG, E, ...) FN(ARG, E) FOREACHA_12(FN, ARG, __VA_ARGS__)
#define FOREACHA_14(FN, ARG, E, ...) FN(ARG, E) FOREACHA_13(FN, ARG, __VA_ARGS__)
#define FOREACHA_15(FN, ARG, E, ...) FN(ARG, E) FOREACHA_14(FN, ARG, __VA_ARGS__)
#define FOREACHA_16(FN, ARG, E, ...) FN(ARG, E) FOREACHA_15(FN, ARG, __VA_ARGS__)
#define FOREACHA_17(FN, ARG, E, ...) FN(ARG, E) FOREACHA_16(FN, ARG, __VA_ARGS__)
#define FOREACHA_18(FN, ARG, E, ...) FN(ARG, E) FOREACHA_17(FN, ARG, __VA_ARGS__)
#define FOREACHA_19(FN, ARG, E, ...) FN(ARG, E) FOREACHA_18(FN, ARG, __VA_ARGS__)
#define FOREACHA_20(FN, ARG, E, ...) FN(ARG, E) FOREACHA_19(FN, ARG, __VA_ARGS__)
#define FOREACHA_21(FN, ARG, E, ...) FN(ARG, E) FOREACHA_20(FN, ARG, __VA_ARGS__)
#define FOREACHA_22(FN, ARG, E, ...) FN(ARG, E) FOREACHA_21(FN, ARG, __VA_ARGS__)
#define FOREACHA_23(FN, ARG, E, ...) FN(ARG, E) FOREACHA_22(FN, ARG, __VA_ARGS__)
#define FOREACHA_24(FN, ARG, E, ...) FN(ARG, E) FOREACHA_23(FN, ARG, __VA_ARGS__)
#define FOREACHA_25(FN, ARG, E, ...) FN(ARG, E) FOREACHA_24(FN, ARG, __VA_ARGS__)
#define FOREACHA_26(FN, ARG, E, ...) FN(ARG, E) FOREACHA_25(FN, ARG, __VA_ARGS__)
#define FOREACHA_27(FN, ARG, E, ...) FN(ARG, E) FOREACHA_26(FN, ARG, __VA_ARGS__)
#define FOREACHA_28(FN, ARG, E, ...) FN(ARG, E) FOREACHA_27(FN, ARG, __VA_ARGS__)
#define FOREACHA_29(FN, ARG, E, ...) FN(ARG, E) FOREACHA_28(FN, ARG, __VA_ARGS__)
#define FOREACHA_30(FN, ARG, E, ...) FN(ARG, E) FOREACHA_29(FN, ARG, __VA_ARGS__)
#define FOREACHA_31(FN, ARG, E, ...) FN(ARG, E) FOREACHA_30(FN, ARG, __VA_ARGS__)
#define FOREACHA_32(FN, ARG, E, ...) FN(ARG, E) FOREACHA_31(FN, ARG, __VA_ARGS__)
#define FOREACHA_33(FN, ARG, E, ...) FN(ARG, E) FOREACHA_32(FN, ARG, __VA_ARGS__)
#define FOREACHA_34(FN, ARG, E, ...) FN(ARG, E) FOREACHA_33(FN, ARG, __VA_ARGS__)
#define FOREACHA_35(FN, ARG, E, ...) FN(ARG, E) FOREACHA_34(FN, ARG, __VA_ARGS__)
#define FOREACHA_36(FN, ARG, E, ...) FN(ARG, E) FOREACHA_35(FN, ARG, __VA_ARGS__)
#define FOREACHA_37(FN, ARG, E, ...) FN(ARG, E) FOREACHA_36(FN, ARG, __VA_ARGS__)
#define FOREACHA_38(FN, ARG, E, ...) FN(ARG, E) FOREACHA_37(FN, ARG, __VA_ARGS__)
#define FOREACHA_39(FN, ARG, E, ...) FN(ARG, E) FOREACHA_38(FN, ARG, __VA_ARGS__)
#define FOREACHA_40(FN, ARG, E, ...) FN(ARG, E) FOREACHA_39(FN, ARG, __VA_ARGS__)
#define FOREACHA_41(FN, ARG, E, ...) FN(ARG, E) FOREACHA_40(FN, ARG, __VA_ARGS__)
#define FOREACHA_42(FN, ARG, E, ...) FN(ARG, E) FOREACHA_41(FN, ARG, __VA_ARGS__)
#define FOREACHA_43(FN, ARG, E, ...) FN(ARG, E) FOREACHA_42(FN, ARG, __VA_ARGS__)
#define FOREACHA_44(FN, ARG, E, ...) FN(ARG, E) FOREACHA_43(FN, ARG, __VA_ARGS__)
#define FOREACHA_45(FN, ARG, E, ...) FN(ARG, E) FOREACHA_44(FN, ARG, __VA_ARGS__)
#define FOREACHA_46(FN, ARG, E, ...) FN(ARG, E) FOREACHA_45(FN, ARG, __VA_ARGS__)
#define FOREACHA_47(FN, ARG, E, ...) FN(ARG, E) FOREACHA_46(FN, ARG, __VA_ARGS__)
#define FOREACHA_48(FN, ARG, E, ...) FN(ARG, E) FOREACHA_47(FN, ARG, __VA_ARGS__)
#define FOREACHA_49(FN, ARG, E, ...) FN(ARG, E) FOREACHA_48(FN, ARG, __VA_ARGS__)
#define FOREACHA_50(FN, ARG, E, ...) FN(ARG, E) FOREACHA_49(FN, ARG, __VA_ARGS__)
#define FOREACHA_51(FN, ARG, E, ...) FN(ARG, E) FOREACHA_50(FN, ARG, __VA_ARGS__)
#define FOREACHA_52(FN, ARG, E, ...) FN(ARG, E) FOREACHA_51(FN, ARG, __VA_ARGS__)
#define FOREACHA_53(FN, ARG, E, ...) FN(ARG, E) FOREACHA_52(FN, ARG, __VA_ARGS__)
#define FOREACHA_54(FN, ARG, E, ...) FN(ARG, E) FOREACHA_53(FN, ARG, __VA_ARGS__)
#define FOREACHA_55(FN, ARG, E, ...) FN(ARG, E) FOREACHA_54(FN, ARG, __VA_ARGS__)
#define FOREACHA_56(FN, ARG, E, ...) FN(ARG, E) FOREACHA_55(FN, ARG, __VA_ARGS__)
#define FOREACHA_57(FN, ARG, E, ...) FN(ARG, E) FOREACHA_56(FN, ARG, __VA_ARGS__)
#define FOREACHA_58(FN, ARG, E, ...) FN(ARG, E) FOREACHA_57(FN, ARG, __VA_ARGS__)
#define FOREACHA_59(FN, ARG, E, ...) FN(ARG, E) FOREACHA_58(FN, ARG, __VA_ARGS__)
#define FOREACHA_60(FN, ARG, E, ...) FN(ARG, E) FOREACHA_59(FN, ARG, __VA_ARGS__)
#define FOREACHA_61(FN, ARG, E, ...) FN(ARG, E) FOREACHA_60(FN, ARG, __VA_ARGS__)
#define FOREACHA_62(FN, ARG, E, ...) FN(ARG, E) FOREACHA_61(FN, ARG, __VA_ARGS__)
#define FOREACHA_63(FN, ARG, E, ...) FN(ARG, E) FOREACHA_62(FN, ARG, __VA_ARGS__)
#define FOREACHA_64(FN, ARG, E, ...) FN(ARG, E) FOREACHA_63(FN, ARG, __VA_ARGS__)
#define FOREACHA_65(FN, ARG, E, ...) FN(ARG, E) FOREACHA_64(FN, ARG, __VA_ARGS__)
#define FOREACHA_66(FN, ARG, E, ...) FN(ARG, E) FOREACHA_65(FN, ARG, __VA_ARGS__)
#define FOREACHA_67(FN, ARG, E, ...) FN(ARG, E) FOREACHA_66(FN, ARG, __VA_ARGS__)
#define FOREACHA_68(FN, ARG, E, ...) FN(ARG, E) FOREACHA_67(FN, ARG, __VA_ARGS__)
#define FOREACHA_69(FN, ARG, E, ...) FN(ARG, E) FOREACHA_68(FN, ARG, __VA_ARGS__)
#define FOREACHA_70(FN, ARG, E, ...) FN(ARG, E) FOREACHA_69(FN, ARG, __VA_ARGS__)
#define FOREACHA_71(FN, ARG, E, ...) FN(ARG, E) FOREACHA_70(FN, ARG, __VA_ARGS__)
#define FOREACHA_72(FN, ARG, E, ...) FN(ARG, E) FOREACHA_71(FN, ARG, __VA_ARGS__)
#define FOREACHA_73(FN, ARG, E, ...) FN(ARG, E) FOREACHA_72(FN, ARG, __VA_ARGS__)
#define FOREACHA_74(FN, ARG, E, ...) FN(ARG, E) FOREACHA_73(FN, ARG, __VA_ARGS__)
#define FOREACHA_75(FN, ARG, E, ...) FN(ARG, E) FOREACHA_74(FN, ARG, __VA_ARGS__)
#define FOREACHA_76(FN, ARG, E, ...) FN(ARG, E) FOREACHA_75(FN, ARG, __VA_ARGS__)
#define FOREACHA_77(FN, ARG, E, ...) FN(ARG, E) FOREACHA_76(FN, ARG, __VA_ARGS__)
#define FOREACHA_78(FN, ARG, E, ...) FN(ARG, E) FOREACHA_77(FN, ARG, __VA_ARGS__)
#define FOREACHA_79(FN, ARG, E, ...) FN(ARG, E) FOREACHA_78(FN, ARG, __VA_ARGS__)
#define FOREACHA_80(FN, ARG, E, ...) FN(ARG, E) FOREACHA_79(FN, ARG, __VA_ARGS__)
#define FOREACHA_81(FN, ARG, E, ...) FN(ARG, E) FOREACHA_80(FN, ARG, __VA_ARGS__)
#define FOREACHA_82(FN, ARG, E, ...) FN(ARG, E) FOREACHA_81(FN, ARG, __VA_ARGS__)
#define FOREACHA_83(FN, ARG, E, ...) FN(ARG, E) FOREACHA_82(FN, ARG, __VA_ARGS__)
#define FOREACHA_84(FN, ARG, E, ...) FN(ARG, E) FOREACHA_83(FN, ARG, __VA_ARGS__)
#define FOREACHA_85(FN, ARG, E, ...) FN(ARG, E) FOREACHA_84(FN, ARG, __VA_ARGS__)
#define FOREACHA_86(FN, ARG, E, ...) FN(ARG, E) FOREACHA_85(FN, ARG, __VA_ARGS__)
#define FOREACHA_87(FN, ARG, E, ...) FN(ARG, E) FOREACHA_86(FN, ARG, __VA_ARGS__)
#define FOREACHA_88(FN, ARG, E, ...) FN(ARG, E) FOREACHA_87(FN, ARG, __VA_ARGS__)
#define FOREACHA_89(FN, ARG, E, ...) FN(ARG, E) FOREACHA_88(FN, ARG, __VA_ARGS__)
#define FOREACHA_90(FN, ARG, E, ...) FN(ARG, E) FOREACHA_89(FN, ARG, __VA_ARGS__)
#define FOREACHA_91(FN, ARG, E, ...) FN(ARG, E) FOREACHA_90(FN, ARG, __VA_ARGS__)
#define FOREACHA_92(FN, ARG, E, ...) FN(ARG, E) FOREACHA_91(FN, ARG, __VA_ARGS__)
#define FOREACHA_93(FN, ARG, E, ...) FN(ARG, E) FOREACHA_92(FN, ARG, __VA_ARGS__)
#define FOREACHA_94(FN, ARG, E, ...) FN(ARG, E) FOREACHA_93(FN, ARG, __VA_ARGS__)
#define FOREACHA_95(FN, ARG, E, ...) FN(ARG, E) FOREACHA_94(FN, ARG, __VA_ARGS__)
#define FOREACHA_96(FN, ARG, E, ...) FN(ARG, E) FOREACHA_95(FN, ARG, __VA_ARGS__)
#define FOREACHA_97(FN, ARG, E, ...) FN(ARG, E) FOREACHA_96(FN, ARG, __VA_ARGS__)
#define FOREACHA_98(FN, ARG, E, ...) FN(ARG, E) FOREACHA_97(FN, ARG, __VA_ARGS__)
#define FOREACHA_99(FN, ARG, E, ...) FN(ARG, E) FOREACHA_98(FN, ARG, __VA_ARGS__)
#define FOREACHA_100(FN, ARG, E, ...) FN(ARG, E) FOREACHA_99(FN, ARG, __VA_ARGS__)

#define FOREACHA__(FN, ARG, NARGS, ...) FOREACHA_##NARGS(FN, ARG, __VA_ARGS__) 
#define FOREACHA_(FN, ARG, NARGS, ...) FOREACHA__(FN, ARG, NARGS, __VA_ARGS__)
#define FOREACHA(FN, ARG, ...) FOREACHA_(FN, ARG, N_VA_ARGS(__VA_ARGS__), __VA_ARGS__)