#include "H264or5VideoStreamFramer.hh"
#include "MPEGVideoStreamParser.hh"
#include "BitVector.hh"
class H264or5VideoStreamParser : public MPEGVideoStreamParser
{
public:
    H264or5VideoStreamParser(int hNumber, H264or5VideoStreamFramer* usingSource,
                             FramedSource* inputSource, Boolean includeStartCodeInOutput);
    virtual ~H264or5VideoStreamParser();
private:
    virtual void flushInput();
    virtual unsigned parse();
private:
    H264or5VideoStreamFramer * usingSource()
    {
        return (H264or5VideoStreamFramer *) fUsingSource;
    }

    Boolean isVPS(u_int8_t nal_unit_type)
    {
        return usingSource()->isVPS(nal_unit_type);
    }

    Boolean isSPS(u_int8_t nal_unit_type)
    {
        return usingSource()->isSPS(nal_unit_type);
    }

    Boolean isPPS(u_int8_t nal_unit_type)
    {
        return usingSource()->isPPS(nal_unit_type);
    }

    Boolean isVCL(u_int8_t nal_unit_type)
    {
        return usingSource()->isVCL(nal_unit_type);
    }

    Boolean isSEI(u_int8_t nal_unit_type);
    Boolean isEOF(u_int8_t nal_unit_type);
    Boolean usuallyBeginsAccessUnit(u_int8_t nal_unit_type);
    void removeEmulationBytes(u_int8_t* nalUnitCopy, unsigned maxSize, unsigned& nalUnitCopySize);
    void analyze_video_parameter_set_data(unsigned& num_units_in_tick, unsigned& time_scale);
    void analyze_seq_parameter_set_data(unsigned& num_units_in_tick, unsigned& time_scale);
    void profile_tier_level(BitVector& bv, unsigned max_sub_layers_minus1);
    void analyze_vui_parameters(BitVector& bv, unsigned& num_units_in_tick, unsigned& time_scale);
    void analyze_sei_data(u_int8_t nal_unit_type);
private:
    int fHNumber;
    unsigned fOutputStartCodeSize;
    Boolean fHaveSeenFirstStartCode, fHaveSeenFirstByteOfNALUnit;
    u_int8_t fFirstByteOfNALUnit;
    double fParsedFrameRate;
};
H264or5VideoStreamFramer
::H264or5VideoStreamFramer(int hNumber, UsageEnvironment& env, FramedSource* inputSource,
                           Boolean createParser, Boolean includeStartCodeInOutput)
    : MPEGVideoStreamFramer(env, inputSource),
    fHNumber(hNumber),
    fLastSeenVPS(NULL), fLastSeenVPSSize(0),
    fLastSeenSPS(NULL), fLastSeenSPSSize(0),
    fLastSeenPPS(NULL), fLastSeenPPSSize(0)
{
    fParser = createParser ?
              new H264or5VideoStreamParser(hNumber, this, inputSource, includeStartCodeInOutput) :
              NULL;
    fNextPresentationTime = fPresentationTimeBase;
    fFrameRate = 25.0;
}

H264or5VideoStreamFramer::~H264or5VideoStreamFramer()
{
    delete[] fLastSeenPPS;
    delete[] fLastSeenSPS;
    delete[] fLastSeenVPS;
}

#define VPS_MAX_SIZE 1000
void H264or5VideoStreamFramer::saveCopyOfVPS(u_int8_t* from, unsigned size)
{
    if (from == NULL) {
        return;
    }
    delete[] fLastSeenVPS;
    fLastSeenVPS = new u_int8_t[size];
    memmove(fLastSeenVPS, from, size);
    fLastSeenVPSSize = size;
}

#define SPS_MAX_SIZE 1000
void H264or5VideoStreamFramer::saveCopyOfSPS(u_int8_t* from, unsigned size)
{
    if (from == NULL) {
        return;
    }
    delete[] fLastSeenSPS;
    fLastSeenSPS = new u_int8_t[size];
    memmove(fLastSeenSPS, from, size);
    fLastSeenSPSSize = size;
}

void H264or5VideoStreamFramer::saveCopyOfPPS(u_int8_t* from, unsigned size)
{
    if (from == NULL) {
        return;
    }
    delete[] fLastSeenPPS;
    fLastSeenPPS = new u_int8_t[size];
    memmove(fLastSeenPPS, from, size);
    fLastSeenPPSSize = size;
}

Boolean H264or5VideoStreamFramer::isVPS(u_int8_t nal_unit_type)
{
    return fHNumber == 265 && nal_unit_type == 32;
}

Boolean H264or5VideoStreamFramer::isSPS(u_int8_t nal_unit_type)
{
    return fHNumber == 264 ? nal_unit_type == 7 : nal_unit_type == 33;
}

Boolean H264or5VideoStreamFramer::isPPS(u_int8_t nal_unit_type)
{
    return fHNumber == 264 ? nal_unit_type == 8 : nal_unit_type == 34;
}

Boolean H264or5VideoStreamFramer::isVCL(u_int8_t nal_unit_type)
{
    return fHNumber == 264 ?
           (nal_unit_type <= 5 && nal_unit_type > 0) :
           (nal_unit_type <= 31);
}

H264or5VideoStreamParser
::H264or5VideoStreamParser(int hNumber, H264or5VideoStreamFramer* usingSource,
                           FramedSource* inputSource, Boolean includeStartCodeInOutput)
    : MPEGVideoStreamParser(usingSource, inputSource),
    fHNumber(hNumber), fOutputStartCodeSize(includeStartCodeInOutput ? 4 : 0), fHaveSeenFirstStartCode(False),
    fHaveSeenFirstByteOfNALUnit(False), fParsedFrameRate(0.0)
{}

H264or5VideoStreamParser::~H264or5VideoStreamParser()
{}

#define PREFIX_SEI_NUT 39
#define SUFFIX_SEI_NUT 40
Boolean H264or5VideoStreamParser::isSEI(u_int8_t nal_unit_type)
{
    return fHNumber == 264 ?
           nal_unit_type == 6 :
           (nal_unit_type == PREFIX_SEI_NUT || nal_unit_type == SUFFIX_SEI_NUT);
}

Boolean H264or5VideoStreamParser::isEOF(u_int8_t nal_unit_type)
{
    return fHNumber == 264 ?
           (nal_unit_type == 10 || nal_unit_type == 11) :
           (nal_unit_type == 36 || nal_unit_type == 37);
}

Boolean H264or5VideoStreamParser::usuallyBeginsAccessUnit(u_int8_t nal_unit_type)
{
    return fHNumber == 264 ?
           (nal_unit_type >= 6 && nal_unit_type <= 9) || (nal_unit_type >= 14 && nal_unit_type <= 18) :
           (nal_unit_type >= 32 && nal_unit_type <= 35) || (nal_unit_type == 39) ||
           (nal_unit_type >= 41 && nal_unit_type <= 44) ||
           (nal_unit_type >= 48 && nal_unit_type <= 55);
}

void H264or5VideoStreamParser
::removeEmulationBytes(u_int8_t* nalUnitCopy, unsigned maxSize, unsigned& nalUnitCopySize)
{
    u_int8_t* nalUnitOrig = fStartOfFrame + fOutputStartCodeSize;
    unsigned const numBytesInNALunit = fTo - nalUnitOrig;

    nalUnitCopySize =
        removeH264or5EmulationBytes(nalUnitCopy, maxSize, nalUnitOrig, numBytesInNALunit);
}

#ifdef DEBUG
char const* nal_unit_type_description_h264[32] = {
    "Unspecified",
    "Coded slice of a non-IDR picture",
    "Coded slice data partition A",
    "Coded slice data partition B",
    "Coded slice data partition C",
    "Coded slice of an IDR picture",
    "Supplemental enhancement information (SEI)",
    "Sequence parameter set",
    "Picture parameter set",
    "Access unit delimiter",
    "End of sequence",
    "End of stream",
    "Filler data",
    "Sequence parameter set extension",
    "Prefix NAL unit",
    "Subset sequence parameter set",
    "Reserved",
    "Reserved",
    "Reserved",
    "Coded slice of an auxiliary coded picture without partitioning",
    "Coded slice extension",
    "Reserved",
    "Reserved",
    "Reserved",
    "Unspecified",
    "Unspecified",
    "Unspecified",
    "Unspecified",
    "Unspecified",
    "Unspecified",
    "Unspecified",
    "Unspecified"
};
char const* nal_unit_type_description_h265[64] = {
    "Coded slice segment of a non-TSA, non-STSA trailing picture",
    "Coded slice segment of a non-TSA, non-STSA trailing picture",
    "Coded slice segment of a TSA picture",
    "Coded slice segment of a TSA picture",
    "Coded slice segment of a STSA picture",
    "Coded slice segment of a STSA picture",
    "Coded slice segment of a RADL picture",
    "Coded slice segment of a RADL picture",
    "Coded slice segment of a RASL picture",
    "Coded slice segment of a RASL picture",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Coded slice segment of a BLA picture",
    "Coded slice segment of a BLA picture",
    "Coded slice segment of a BLA picture",
    "Coded slice segment of an IDR picture",
    "Coded slice segment of an IDR picture",
    "Coded slice segment of a CRA picture",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Video parameter set",
    "Sequence parameter set",
    "Picture parameter set",
    "Access unit delimiter",
    "End of sequence",
    "End of bitstream",
    "Filler data",
    "Supplemental enhancement information (SEI)",
    "Supplemental enhancement information (SEI)",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Unspecified",
    "Unspecified",
    "Unspecified",
    "Unspecified",
    "Unspecified",
    "Unspecified",
    "Unspecified",
    "Unspecified",
    "Unspecified",
    "Unspecified",
    "Unspecified",
    "Unspecified",
    "Unspecified",
    "Unspecified",
    "Unspecified",
    "Unspecified",
};
#endif // ifdef DEBUG
#ifdef DEBUG
static unsigned numDebugTabs = 1;
# define DEBUG_PRINT_TABS for (unsigned _i = 0; _i < numDebugTabs; ++_i) fprintf (stderr, "\t")
# define DEBUG_PRINT(x) do { DEBUG_PRINT_TABS; fprintf(stderr, "%s: %d\n", #x, x); } while (0)
# define DEBUG_STR(x)   do { DEBUG_PRINT_TABS; fprintf(stderr, "%s\n", x); } while (0)
class DebugTab
{
public:
    DebugTab()
    {
        ++numDebugTabs;
    }

    ~DebugTab()
    {
        --numDebugTabs;
    }
};
# define DEBUG_TAB DebugTab dummy
#else // ifdef DEBUG
# define DEBUG_PRINT(x) do { x = x; } while (0)
# define DEBUG_STR(x)   do {} while (0)
# define DEBUG_TAB do {} while (0)
#endif // ifdef DEBUG
void H264or5VideoStreamParser::profile_tier_level(BitVector& bv, unsigned max_sub_layers_minus1)
{
    bv.skipBits(96);
    unsigned i;
    Boolean sub_layer_profile_present_flag[7], sub_layer_level_present_flag[7];
    for (i = 0; i < max_sub_layers_minus1; ++i) {
        sub_layer_profile_present_flag[i] = bv.get1BitBoolean();
        sub_layer_level_present_flag[i]   = bv.get1BitBoolean();
    }
    if (max_sub_layers_minus1 > 0) {
        bv.skipBits(2 * (8 - max_sub_layers_minus1));
    }
    for (i = 0; i < max_sub_layers_minus1; ++i) {
        if (sub_layer_profile_present_flag[i]) {
            bv.skipBits(88);
        }
        if (sub_layer_level_present_flag[i]) {
            bv.skipBits(8);
        }
    }
}

void H264or5VideoStreamParser
::analyze_vui_parameters(BitVector& bv,
                         unsigned& num_units_in_tick, unsigned& time_scale)
{
    Boolean aspect_ratio_info_present_flag = bv.get1BitBoolean();

    DEBUG_PRINT(aspect_ratio_info_present_flag);
    if (aspect_ratio_info_present_flag) {
        DEBUG_TAB;
        unsigned aspect_ratio_idc = bv.getBits(8);
        DEBUG_PRINT(aspect_ratio_idc);
        if (aspect_ratio_idc == 255) {
            bv.skipBits(32);
        }
    }
    Boolean overscan_info_present_flag = bv.get1BitBoolean();
    DEBUG_PRINT(overscan_info_present_flag);
    if (overscan_info_present_flag) {
        bv.skipBits(1);
    }
    Boolean video_signal_type_present_flag = bv.get1BitBoolean();
    DEBUG_PRINT(video_signal_type_present_flag);
    if (video_signal_type_present_flag) {
        DEBUG_TAB;
        bv.skipBits(4);
        Boolean colour_description_present_flag = bv.get1BitBoolean();
        DEBUG_PRINT(colour_description_present_flag);
        if (colour_description_present_flag) {
            bv.skipBits(24);
        }
    }
    Boolean chroma_loc_info_present_flag = bv.get1BitBoolean();
    DEBUG_PRINT(chroma_loc_info_present_flag);
    if (chroma_loc_info_present_flag) {
        (void) bv.get_expGolomb();
        (void) bv.get_expGolomb();
    }
    if (fHNumber == 265) {
        bv.skipBits(3);
        Boolean default_display_window_flag = bv.get1BitBoolean();
        DEBUG_PRINT(default_display_window_flag);
        if (default_display_window_flag) {
            (void) bv.get_expGolomb();
            (void) bv.get_expGolomb();
            (void) bv.get_expGolomb();
            (void) bv.get_expGolomb();
        }
    }
    Boolean timing_info_present_flag = bv.get1BitBoolean();
    DEBUG_PRINT(timing_info_present_flag);
    if (timing_info_present_flag) {
        DEBUG_TAB;
        num_units_in_tick = bv.getBits(32);
        DEBUG_PRINT(num_units_in_tick);
        time_scale = bv.getBits(32);
        DEBUG_PRINT(time_scale);
        if (fHNumber == 264) {
            Boolean fixed_frame_rate_flag = bv.get1BitBoolean();
            DEBUG_PRINT(fixed_frame_rate_flag);
        } else {
            Boolean vui_poc_proportional_to_timing_flag = bv.get1BitBoolean();
            DEBUG_PRINT(vui_poc_proportional_to_timing_flag);
            if (vui_poc_proportional_to_timing_flag) {
                unsigned vui_num_ticks_poc_diff_one_minus1 = bv.get_expGolomb();
                DEBUG_PRINT(vui_num_ticks_poc_diff_one_minus1);
            }
        }
    }
} // H264or5VideoStreamParser::analyze_vui_parameters

void H264or5VideoStreamParser
::analyze_video_parameter_set_data(unsigned& num_units_in_tick, unsigned& time_scale)
{
    num_units_in_tick = time_scale = 0;
    u_int8_t vps[VPS_MAX_SIZE];
    unsigned vpsSize;
    removeEmulationBytes(vps, sizeof vps, vpsSize);
    BitVector bv(vps, 0, 8 * vpsSize);
    unsigned i;
    bv.skipBits(28);
    unsigned vps_max_sub_layers_minus1 = bv.getBits(3);
    DEBUG_PRINT(vps_max_sub_layers_minus1);
    bv.skipBits(17);
    profile_tier_level(bv, vps_max_sub_layers_minus1);
    Boolean vps_sub_layer_ordering_info_present_flag = bv.get1BitBoolean();
    DEBUG_PRINT(vps_sub_layer_ordering_info_present_flag);
    for (i = vps_sub_layer_ordering_info_present_flag ? 0 : vps_max_sub_layers_minus1;
         i <= vps_max_sub_layers_minus1; ++i)
    {
        (void) bv.get_expGolomb();
        (void) bv.get_expGolomb();
        (void) bv.get_expGolomb();
    }
    unsigned vps_max_layer_id = bv.getBits(6);
    DEBUG_PRINT(vps_max_layer_id);
    unsigned vps_num_layer_sets_minus1 = bv.get_expGolomb();
    DEBUG_PRINT(vps_num_layer_sets_minus1);
    for (i = 1; i <= vps_num_layer_sets_minus1; ++i) {
        bv.skipBits(vps_max_layer_id + 1);
    }
    Boolean vps_timing_info_present_flag = bv.get1BitBoolean();
    DEBUG_PRINT(vps_timing_info_present_flag);
    if (vps_timing_info_present_flag) {
        DEBUG_TAB;
        num_units_in_tick = bv.getBits(32);
        DEBUG_PRINT(num_units_in_tick);
        time_scale = bv.getBits(32);
        DEBUG_PRINT(time_scale);
        Boolean vps_poc_proportional_to_timing_flag = bv.get1BitBoolean();
        DEBUG_PRINT(vps_poc_proportional_to_timing_flag);
        if (vps_poc_proportional_to_timing_flag) {
            unsigned vps_num_ticks_poc_diff_one_minus1 = bv.get_expGolomb();
            DEBUG_PRINT(vps_num_ticks_poc_diff_one_minus1);
        }
    }
    Boolean vps_extension_flag = bv.get1BitBoolean();
    DEBUG_PRINT(vps_extension_flag);
} // H264or5VideoStreamParser::analyze_video_parameter_set_data

void H264or5VideoStreamParser
::analyze_seq_parameter_set_data(unsigned& num_units_in_tick, unsigned& time_scale)
{
    num_units_in_tick = time_scale = 0;
    u_int8_t sps[SPS_MAX_SIZE];
    unsigned spsSize;
    removeEmulationBytes(sps, sizeof sps, spsSize);
    BitVector bv(sps, 0, 8 * spsSize);
    if (fHNumber == 264) {
        bv.skipBits(8);
        unsigned profile_idc = bv.getBits(8);
        DEBUG_PRINT(profile_idc);
        unsigned constraint_setN_flag = bv.getBits(8);
        DEBUG_PRINT(constraint_setN_flag);
        unsigned level_idc = bv.getBits(8);
        DEBUG_PRINT(level_idc);
        unsigned seq_parameter_set_id = bv.get_expGolomb();
        DEBUG_PRINT(seq_parameter_set_id);
        if (profile_idc == 100 || profile_idc == 110 || profile_idc == 122 || profile_idc == 244 || profile_idc == 44 ||
            profile_idc == 83 || profile_idc == 86 || profile_idc == 118 || profile_idc == 128)
        {
            DEBUG_TAB;
            unsigned chroma_format_idc = bv.get_expGolomb();
            DEBUG_PRINT(chroma_format_idc);
            if (chroma_format_idc == 3) {
                DEBUG_TAB;
                Boolean separate_colour_plane_flag = bv.get1BitBoolean();
                DEBUG_PRINT(separate_colour_plane_flag);
            }
            (void) bv.get_expGolomb();
            (void) bv.get_expGolomb();
            bv.skipBits(1);
            Boolean seq_scaling_matrix_present_flag = bv.get1BitBoolean();
            DEBUG_PRINT(seq_scaling_matrix_present_flag);
            if (seq_scaling_matrix_present_flag) {
                for (int i = 0; i < ((chroma_format_idc != 3) ? 8 : 12); ++i) {
                    DEBUG_TAB;
                    DEBUG_PRINT(i);
                    Boolean seq_scaling_list_present_flag = bv.get1BitBoolean();
                    DEBUG_PRINT(seq_scaling_list_present_flag);
                    if (seq_scaling_list_present_flag) {
                        DEBUG_TAB;
                        unsigned sizeOfScalingList = i < 6 ? 16 : 64;
                        unsigned lastScale         = 8;
                        unsigned nextScale         = 8;
                        for (unsigned j = 0; j < sizeOfScalingList; ++j) {
                            DEBUG_TAB;
                            DEBUG_PRINT(j);
                            DEBUG_PRINT(nextScale);
                            if (nextScale != 0) {
                                DEBUG_TAB;
                                unsigned delta_scale = bv.get_expGolomb();
                                DEBUG_PRINT(delta_scale);
                                nextScale = (lastScale + delta_scale + 256) % 256;
                            }
                            lastScale = (nextScale == 0) ? lastScale : nextScale;
                            DEBUG_PRINT(lastScale);
                        }
                    }
                }
            }
        }
        unsigned log2_max_frame_num_minus4 = bv.get_expGolomb();
        DEBUG_PRINT(log2_max_frame_num_minus4);
        unsigned pic_order_cnt_type = bv.get_expGolomb();
        DEBUG_PRINT(pic_order_cnt_type);
        if (pic_order_cnt_type == 0) {
            DEBUG_TAB;
            unsigned log2_max_pic_order_cnt_lsb_minus4 = bv.get_expGolomb();
            DEBUG_PRINT(log2_max_pic_order_cnt_lsb_minus4);
        } else if (pic_order_cnt_type == 1) {
            DEBUG_TAB;
            bv.skipBits(1);
            (void) bv.get_expGolomb();
            (void) bv.get_expGolomb();
            unsigned num_ref_frames_in_pic_order_cnt_cycle = bv.get_expGolomb();
            DEBUG_PRINT(num_ref_frames_in_pic_order_cnt_cycle);
            for (unsigned i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; ++i) {
                (void) bv.get_expGolomb();
            }
        }
        unsigned max_num_ref_frames = bv.get_expGolomb();
        DEBUG_PRINT(max_num_ref_frames);
        Boolean gaps_in_frame_num_value_allowed_flag = bv.get1BitBoolean();
        DEBUG_PRINT(gaps_in_frame_num_value_allowed_flag);
        unsigned pic_width_in_mbs_minus1 = bv.get_expGolomb();
        DEBUG_PRINT(pic_width_in_mbs_minus1);
        unsigned pic_height_in_map_units_minus1 = bv.get_expGolomb();
        DEBUG_PRINT(pic_height_in_map_units_minus1);
        Boolean frame_mbs_only_flag = bv.get1BitBoolean();
        DEBUG_PRINT(frame_mbs_only_flag);
        if (!frame_mbs_only_flag) {
            bv.skipBits(1);
        }
        bv.skipBits(1);
        Boolean frame_cropping_flag = bv.get1BitBoolean();
        DEBUG_PRINT(frame_cropping_flag);
        if (frame_cropping_flag) {
            (void) bv.get_expGolomb();
            (void) bv.get_expGolomb();
            (void) bv.get_expGolomb();
            (void) bv.get_expGolomb();
        }
        Boolean vui_parameters_present_flag = bv.get1BitBoolean();
        DEBUG_PRINT(vui_parameters_present_flag);
        if (vui_parameters_present_flag) {
            DEBUG_TAB;
            analyze_vui_parameters(bv, num_units_in_tick, time_scale);
        }
    } else {
        unsigned i;
        bv.skipBits(16);
        bv.skipBits(4);
        unsigned sps_max_sub_layers_minus1 = bv.getBits(3);
        DEBUG_PRINT(sps_max_sub_layers_minus1);
        bv.skipBits(1);
        profile_tier_level(bv, sps_max_sub_layers_minus1);
        (void) bv.get_expGolomb();
        unsigned chroma_format_idc = bv.get_expGolomb();
        DEBUG_PRINT(chroma_format_idc);
        if (chroma_format_idc == 3) {
            bv.skipBits(1);
        }
        unsigned pic_width_in_luma_samples = bv.get_expGolomb();
        DEBUG_PRINT(pic_width_in_luma_samples);
        unsigned pic_height_in_luma_samples = bv.get_expGolomb();
        DEBUG_PRINT(pic_height_in_luma_samples);
        Boolean conformance_window_flag = bv.get1BitBoolean();
        DEBUG_PRINT(conformance_window_flag);
        if (conformance_window_flag) {
            DEBUG_TAB;
            unsigned conf_win_left_offset = bv.get_expGolomb();
            DEBUG_PRINT(conf_win_left_offset);
            unsigned conf_win_right_offset = bv.get_expGolomb();
            DEBUG_PRINT(conf_win_right_offset);
            unsigned conf_win_top_offset = bv.get_expGolomb();
            DEBUG_PRINT(conf_win_top_offset);
            unsigned conf_win_bottom_offset = bv.get_expGolomb();
            DEBUG_PRINT(conf_win_bottom_offset);
        }
        (void) bv.get_expGolomb();
        (void) bv.get_expGolomb();
        unsigned log2_max_pic_order_cnt_lsb_minus4       = bv.get_expGolomb();
        Boolean sps_sub_layer_ordering_info_present_flag = bv.get1BitBoolean();
        DEBUG_PRINT(sps_sub_layer_ordering_info_present_flag);
        for (i = (sps_sub_layer_ordering_info_present_flag ? 0 : sps_max_sub_layers_minus1);
             i <= sps_max_sub_layers_minus1; ++i)
        {
            (void) bv.get_expGolomb();
            (void) bv.get_expGolomb();
            (void) bv.get_expGolomb();
        }
        (void) bv.get_expGolomb();
        (void) bv.get_expGolomb();
        (void) bv.get_expGolomb();
        (void) bv.get_expGolomb();
        (void) bv.get_expGolomb();
        (void) bv.get_expGolomb();
        Boolean scaling_list_enabled_flag = bv.get1BitBoolean();
        DEBUG_PRINT(scaling_list_enabled_flag);
        if (scaling_list_enabled_flag) {
            DEBUG_TAB;
            Boolean sps_scaling_list_data_present_flag = bv.get1BitBoolean();
            DEBUG_PRINT(sps_scaling_list_data_present_flag);
            if (sps_scaling_list_data_present_flag) {
                DEBUG_TAB;
                for (unsigned sizeId = 0; sizeId < 4; ++sizeId) {
                    DEBUG_PRINT(sizeId);
                    for (unsigned matrixId = 0; matrixId < (sizeId == 3 ? 2 : 6); ++matrixId) {
                        DEBUG_TAB;
                        DEBUG_PRINT(matrixId);
                        Boolean scaling_list_pred_mode_flag = bv.get1BitBoolean();
                        DEBUG_PRINT(scaling_list_pred_mode_flag);
                        if (!scaling_list_pred_mode_flag) {
                            (void) bv.get_expGolomb();
                        } else {
                            unsigned const c = 1 << (4 + (sizeId << 1));
                            unsigned coefNum = c < 64 ? c : 64;
                            if (sizeId > 1) {
                                (void) bv.get_expGolomb();
                            }
                            for (i = 0; i < coefNum; ++i) {
                                (void) bv.get_expGolomb();
                            }
                        }
                    }
                }
            }
        }
        bv.skipBits(2);
        Boolean pcm_enabled_flag = bv.get1BitBoolean();
        DEBUG_PRINT(pcm_enabled_flag);
        if (pcm_enabled_flag) {
            bv.skipBits(8);
            (void) bv.get_expGolomb();
            (void) bv.get_expGolomb();
            bv.skipBits(1);
        }
        unsigned num_short_term_ref_pic_sets = bv.get_expGolomb();
        DEBUG_PRINT(num_short_term_ref_pic_sets);
        unsigned num_negative_pics = 0, prev_num_negative_pics = 0;
        unsigned num_positive_pics = 0, prev_num_positive_pics = 0;
        for (i = 0; i < num_short_term_ref_pic_sets; ++i) {
            DEBUG_TAB;
            DEBUG_PRINT(i);
            Boolean inter_ref_pic_set_prediction_flag = False;
            if (i != 0) {
                inter_ref_pic_set_prediction_flag = bv.get1BitBoolean();
            }
            DEBUG_PRINT(inter_ref_pic_set_prediction_flag);
            if (inter_ref_pic_set_prediction_flag) {
                DEBUG_TAB;
                if (i == num_short_term_ref_pic_sets) {
                    (void) bv.get_expGolomb();
                }
                bv.skipBits(1);
                (void) bv.get_expGolomb();
                unsigned NumDeltaPocs = prev_num_negative_pics + prev_num_positive_pics;
                for (unsigned j = 0; j < NumDeltaPocs; ++j) {
                    DEBUG_PRINT(j);
                    Boolean used_by_curr_pic_flag = bv.get1BitBoolean();
                    DEBUG_PRINT(used_by_curr_pic_flag);
                    if (!used_by_curr_pic_flag) {
                        bv.skipBits(1);
                    }
                }
            } else {
                prev_num_negative_pics = num_negative_pics;
                num_negative_pics      = bv.get_expGolomb();
                DEBUG_PRINT(num_negative_pics);
                prev_num_positive_pics = num_positive_pics;
                num_positive_pics      = bv.get_expGolomb();
                DEBUG_PRINT(num_positive_pics);
                unsigned k;
                for (k = 0; k < num_negative_pics; ++k) {
                    (void) bv.get_expGolomb();
                    bv.skipBits(1);
                }
                for (k = 0; k < num_positive_pics; ++k) {
                    (void) bv.get_expGolomb();
                    bv.skipBits(1);
                }
            }
        }
        Boolean long_term_ref_pics_present_flag = bv.get1BitBoolean();
        DEBUG_PRINT(long_term_ref_pics_present_flag);
        if (long_term_ref_pics_present_flag) {
            DEBUG_TAB;
            unsigned num_long_term_ref_pics_sps = bv.get_expGolomb();
            DEBUG_PRINT(num_long_term_ref_pics_sps);
            for (i = 0; i < num_long_term_ref_pics_sps; ++i) {
                bv.skipBits(log2_max_pic_order_cnt_lsb_minus4);
                bv.skipBits(1);
            }
        }
        bv.skipBits(2);
        Boolean vui_parameters_present_flag     = bv.get1BitBoolean();
        DEBUG_PRINT(vui_parameters_present_flag);
        if (vui_parameters_present_flag) {
            DEBUG_TAB;
            analyze_vui_parameters(bv, num_units_in_tick, time_scale);
        }
        Boolean sps_extension_flag = bv.get1BitBoolean();
        DEBUG_PRINT(sps_extension_flag);
    }
} // H264or5VideoStreamParser::analyze_seq_parameter_set_data

#define SEI_MAX_SIZE                           5000
#ifdef DEBUG
# define MAX_SEI_PAYLOAD_TYPE_DESCRIPTION_H264 46
char const* sei_payloadType_description_h264[MAX_SEI_PAYLOAD_TYPE_DESCRIPTION_H264 + 1] = {
    "buffering_period",
    "pic_timing",
    "pan_scan_rect",
    "filler_payload",
    "user_data_registered_itu_t_t35",
    "user_data_unregistered",
    "recovery_point",
    "dec_ref_pic_marking_repetition",
    "spare_pic",
    "scene_info",
    "sub_seq_info",
    "sub_seq_layer_characteristics",
    "sub_seq_characteristics",
    "full_frame_freeze",
    "full_frame_freeze_release",
    "full_frame_snapshot",
    "progressive_refinement_segment_start",
    "progressive_refinement_segment_end",
    "motion_constrained_slice_group_set",
    "film_grain_characteristics",
    "deblocking_filter_display_preference",
    "stereo_video_info",
    "post_filter_hint",
    "tone_mapping_info",
    "scalability_info",
    "sub_pic_scalable_layer",
    "non_required_layer_rep",
    "priority_layer_info",
    "layers_not_present",
    "layer_dependency_change",
    "scalable_nesting",
    "base_layer_temporal_hrd",
    "quality_layer_integrity_check",
    "redundant_pic_property",
    "tl0_dep_rep_index",
    "tl_switching_point",
    "parallel_decoding_info",
    "mvc_scalable_nesting",
    "view_scalability_info",
    "multiview_scene_info",
    "multiview_acquisition_info",
    "non_required_view_component",
    "view_dependency_change",
    "operation_points_not_present",
    "base_view_temporal_hrd",
    "frame_packing_arrangement",
    "reserved_sei_message"
};
#endif // ifdef DEBUG
void H264or5VideoStreamParser::analyze_sei_data(u_int8_t nal_unit_type)
{
    u_int8_t sei[SEI_MAX_SIZE];
    unsigned seiSize;

    removeEmulationBytes(sei, sizeof sei, seiSize);
    unsigned j = 1;
    while (j < seiSize) {
        unsigned payloadType = 0;
        do {
            payloadType += sei[j];
        } while (sei[j++] == 255 && j < seiSize);
        if (j >= seiSize) {
            break;
        }
        unsigned payloadSize = 0;
        do {
            payloadSize += sei[j];
        } while (sei[j++] == 255 && j < seiSize);
        if (j >= seiSize) {
            break;
        }
        #ifdef DEBUG
        char const* description;
        if (fHNumber == 264) {
            unsigned descriptionNum = payloadType <= MAX_SEI_PAYLOAD_TYPE_DESCRIPTION_H264 ?
                                      payloadType : MAX_SEI_PAYLOAD_TYPE_DESCRIPTION_H264;
            description = sei_payloadType_description_h264[descriptionNum];
        } else {
            description =
                payloadType == 3 ? "filler_payload" :
                payloadType == 4 ? "user_data_registered_itu_t_t35" :
                payloadType == 5 ? "user_data_unregistered" :
                payloadType == 17 ? "progressive_refinement_segment_end" :
                payloadType == 22 ? "post_filter_hint" :
                (payloadType == 132 && nal_unit_type == SUFFIX_SEI_NUT) ? "decoded_picture_hash" :
                nal_unit_type == SUFFIX_SEI_NUT ? "reserved_sei_message" :
                payloadType == 0 ? "buffering_period" :
                payloadType == 1 ? "pic_timing" :
                payloadType == 2 ? "pan_scan_rect" :
                payloadType == 6 ? "recovery_point" :
                payloadType == 9 ? "scene_info" :
                payloadType == 15 ? "picture_snapshot" :
                payloadType == 16 ? "progressive_refinement_segment_start" :
                payloadType == 19 ? "film_grain_characteristics" :
                payloadType == 23 ? "tone_mapping_info" :
                payloadType == 45 ? "frame_packing_arrangement" :
                payloadType == 47 ? "display_orientation" :
                payloadType == 128 ? "structure_of_pictures_info" :
                payloadType == 129 ? "active_parameter_sets" :
                payloadType == 130 ? "decoding_unit_info" :
                payloadType == 131 ? "temporal_sub_layer_zero_index" :
                payloadType == 133 ? "scalable_nesting" :
                payloadType == 134 ? "region_refresh_info" : "reserved_sei_message";
        }
        fprintf(stderr, "\tpayloadType %d (\"%s\"); payloadSize %d\n", payloadType, description, payloadSize);
        #endif // ifdef DEBUG
        j += payloadSize;
    }
} // H264or5VideoStreamParser::analyze_sei_data

void H264or5VideoStreamParser::flushInput()
{
    fHaveSeenFirstStartCode     = False;
    fHaveSeenFirstByteOfNALUnit = False;
    StreamParser::flushInput();
}

#define NUM_NEXT_SLICE_HEADER_BYTES_TO_ANALYZE 12
unsigned H264or5VideoStreamParser::parse()
{
    try {
        if (!fHaveSeenFirstStartCode) {
            u_int32_t first4Bytes;
            while ((first4Bytes = test4Bytes()) != 0x00000001) {
                get1Byte();
                setParseState();
            }
            skipBytes(4);
            setParseState();
            fHaveSeenFirstStartCode = True;
        }
        if (fOutputStartCodeSize > 0 && curFrameSize() == 0 && !haveSeenEOF()) {
            save4Bytes(0x00000001);
        }
        if (haveSeenEOF()) {
            unsigned remainingDataSize = totNumValidBytes() - curOffset();
            #ifdef DEBUG
            unsigned const trailingNALUnitSize = remainingDataSize;
            #endif
            while (remainingDataSize > 0) {
                u_int8_t nextByte = get1Byte();
                if (!fHaveSeenFirstByteOfNALUnit) {
                    fFirstByteOfNALUnit         = nextByte;
                    fHaveSeenFirstByteOfNALUnit = True;
                }
                saveByte(nextByte);
                --remainingDataSize;
            }
            #ifdef DEBUG
            if (fHNumber == 264) {
                u_int8_t nal_ref_idc   = (fFirstByteOfNALUnit & 0x60) >> 5;
                u_int8_t nal_unit_type = fFirstByteOfNALUnit & 0x1F;
                fprintf(stderr, "Parsed trailing %d-byte NAL-unit (nal_ref_idc: %d, nal_unit_type: %d (\"%s\"))\n",
                        trailingNALUnitSize, nal_ref_idc, nal_unit_type, nal_unit_type_description_h264[nal_unit_type]);
            } else {
                u_int8_t nal_unit_type = (fFirstByteOfNALUnit & 0x7E) >> 1;
                fprintf(stderr, "Parsed trailing %d-byte NAL-unit (nal_unit_type: %d (\"%s\"))\n",
                        trailingNALUnitSize, nal_unit_type, nal_unit_type_description_h265[nal_unit_type]);
            }
            #endif // ifdef DEBUG
            (void) get1Byte();
            return 0;
        } else {
            u_int32_t next4Bytes = test4Bytes();
            if (!fHaveSeenFirstByteOfNALUnit) {
                fFirstByteOfNALUnit         = next4Bytes >> 24;
                fHaveSeenFirstByteOfNALUnit = True;
            }
            while (next4Bytes != 0x00000001 && (next4Bytes & 0xFFFFFF00) != 0x00000100) {
                if ((unsigned) (next4Bytes & 0xFF) > 1) {
                    save4Bytes(next4Bytes);
                    skipBytes(4);
                } else {
                    saveByte(next4Bytes >> 24);
                    skipBytes(1);
                }
                setParseState();
                next4Bytes = test4Bytes();
            }
            if (next4Bytes == 0x00000001) {
                skipBytes(4);
            } else {
                skipBytes(3);
            }
        }
        fHaveSeenFirstByteOfNALUnit = False;
        u_int8_t nal_unit_type;
        if (fHNumber == 264) {
            nal_unit_type = fFirstByteOfNALUnit & 0x1F;
            #ifdef DEBUG
            u_int8_t nal_ref_idc = (fFirstByteOfNALUnit & 0x60) >> 5;
            fprintf(stderr, "Parsed %d-byte NAL-unit (nal_ref_idc: %d, nal_unit_type: %d (\"%s\"))\n",
                    curFrameSize() - fOutputStartCodeSize, nal_ref_idc, nal_unit_type,
                    nal_unit_type_description_h264[nal_unit_type]);
            #endif
        } else {
            nal_unit_type = (fFirstByteOfNALUnit & 0x7E) >> 1;
            #ifdef DEBUG
            fprintf(stderr, "Parsed %d-byte NAL-unit (nal_unit_type: %d (\"%s\"))\n",
                    curFrameSize() - fOutputStartCodeSize, nal_unit_type,
                    nal_unit_type_description_h265[nal_unit_type]);
            #endif
        }
        if (isVPS(nal_unit_type)) {
            usingSource()->saveCopyOfVPS(fStartOfFrame + fOutputStartCodeSize, curFrameSize() - fOutputStartCodeSize);
            if (fParsedFrameRate == 0.0) {
                unsigned num_units_in_tick, time_scale;
                analyze_video_parameter_set_data(num_units_in_tick, time_scale);
                if (time_scale > 0 && num_units_in_tick > 0) {
                    usingSource()->fFrameRate = fParsedFrameRate = time_scale / (2.0 * num_units_in_tick);
                    #ifdef DEBUG
                    fprintf(stderr, "Set frame rate to %f fps\n", usingSource()->fFrameRate);
                    #endif
                } else {
                    #ifdef DEBUG
                    fprintf(stderr,
                            "\tThis \"Video Parameter Set\" NAL unit contained no frame rate information, so we use a default frame rate of %f fps\n",
                            usingSource()->fFrameRate);
                    #endif
                }
            }
        } else if (isSPS(nal_unit_type)) {
            usingSource()->saveCopyOfSPS(fStartOfFrame + fOutputStartCodeSize, curFrameSize() - fOutputStartCodeSize);
            if (fParsedFrameRate == 0.0) {
                unsigned num_units_in_tick, time_scale;
                analyze_seq_parameter_set_data(num_units_in_tick, time_scale);
                if (time_scale > 0 && num_units_in_tick > 0) {
                    usingSource()->fFrameRate = fParsedFrameRate = time_scale / (2.0 * num_units_in_tick);
                    #ifdef DEBUG
                    fprintf(stderr, "Set frame rate to %f fps\n", usingSource()->fFrameRate);
                    #endif
                } else {
                    #ifdef DEBUG
                    fprintf(stderr,
                            "\tThis \"Sequence Parameter Set\" NAL unit contained no frame rate information, so we use a default frame rate of %f fps\n",
                            usingSource()->fFrameRate);
                    #endif
                }
            }
        } else if (isPPS(nal_unit_type)) {
            usingSource()->saveCopyOfPPS(fStartOfFrame + fOutputStartCodeSize, curFrameSize() - fOutputStartCodeSize);
        } else if (isSEI(nal_unit_type)) {
            analyze_sei_data(nal_unit_type);
        }
        usingSource()->setPresentationTime();
        #ifdef DEBUG
        unsigned long secs = (unsigned long) usingSource()->fPresentationTime.tv_sec;
        unsigned uSecs     = (unsigned) usingSource()->fPresentationTime.tv_usec;
        fprintf(stderr, "\tPresentation time: %lu.%06u\n", secs, uSecs);
        #endif
        Boolean thisNALUnitEndsAccessUnit;
        if (haveSeenEOF() || isEOF(nal_unit_type)) {
            thisNALUnitEndsAccessUnit = True;
        } else if (usuallyBeginsAccessUnit(nal_unit_type)) {
            thisNALUnitEndsAccessUnit = False;
        } else {
            u_int8_t firstBytesOfNextNALUnit[3];
            testBytes(firstBytesOfNextNALUnit, 3);
            u_int8_t const& next_nal_unit_type = fHNumber == 264 ?
                                                 (firstBytesOfNextNALUnit[0]
                                                  & 0x1F) : ((firstBytesOfNextNALUnit[0] & 0x7E) >> 1);
            if (isVCL(next_nal_unit_type)) {
                u_int8_t const byteAfter_nal_unit_header =
                    fHNumber == 264 ? firstBytesOfNextNALUnit[1] : firstBytesOfNextNALUnit[2];
                thisNALUnitEndsAccessUnit = (byteAfter_nal_unit_header & 0x80) != 0;
            } else if (usuallyBeginsAccessUnit(next_nal_unit_type)) {
                thisNALUnitEndsAccessUnit = True;
            } else {
                thisNALUnitEndsAccessUnit = False;
            }
        }
        if (thisNALUnitEndsAccessUnit) {
            #ifdef DEBUG
            fprintf(stderr, "*****This NAL unit ends the current access unit*****\n");
            #endif
            usingSource()->fPictureEndMarker = True;
            ++usingSource()->fPictureCount;
            struct timeval& nextPT = usingSource()->fNextPresentationTime;
            nextPT = usingSource()->fPresentationTime;
            double nextFraction        = nextPT.tv_usec / 1000000.0 + 1 / usingSource()->fFrameRate;
            unsigned nextSecsIncrement = (long) nextFraction;
            nextPT.tv_sec += (long) nextSecsIncrement;
            nextPT.tv_usec = (long) ((nextFraction - nextSecsIncrement) * 1000000);
        }
        setParseState();
        return curFrameSize();
    } catch (int) {
        #ifdef DEBUG
        fprintf(stderr, "H264or5VideoStreamParser::parse() EXCEPTION (This is normal behavior - *not* an error)\n");
        #endif
        return 0;
    }
} // H264or5VideoStreamParser::parse

unsigned removeH264or5EmulationBytes(u_int8_t* to, unsigned toMaxSize,
                                     u_int8_t* from, unsigned fromSize)
{
    unsigned toSize = 0;
    unsigned i      = 0;

    while (i < fromSize && toSize + 1 < toMaxSize) {
        if (i + 2 < fromSize && from[i] == 0 && from[i + 1] == 0 && from[i + 2] == 3) {
            to[toSize] = to[toSize + 1] = 0;
            toSize    += 2;
            i += 3;
        } else {
            to[toSize] = from[i];
            toSize    += 1;
            i += 1;
        }
    }
    return toSize;
}
