/* Copyright (C) 2012 Evan Ochsner, 2017 Sebastian Khan
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with with program; see the file COPYING. If not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 */

#include <stdio.h>
#include <lal/LALString.h>
#include <lal/LALSimInspiralWaveformFlags.h>
#include <lal/LALSimInspiralWaveformParams.h>

/**
 * Struct containing several enumerated flags that control specialized behavior
 * for some waveform approximants.
 *
 * Users: Access this struct only through the Create/Destroy/Set/Get/IsDefault
 * functions declared in this file.
 *
 * Developers: Do not add anything but enumerated flags to this struct. Avoid
 * adding extra flags whenever possible.
 * DEPRECATED, use LALDict instead.
 */
struct tagLALSimInspiralWaveformFlags
{
    LALSimInspiralSpinOrder spinO; /**< PN order of spin effects */
    LALSimInspiralTidalOrder tideO; /**< PN order of spin effects */
    LALSimInspiralFrameAxis axisChoice; /**< Flag to set frame z-axis convention */
    LALSimInspiralModesChoice modesChoice; /**< Flag to control which modes are included in IMR models */
    char numreldata[FILENAME_MAX]; /**< Location of NR data file for NR waveforms */
};

/**
 * @addtogroup LALSimInspiralWaveformFlags_c
 * @brief Routines to manipulate inspiral waveform flags structures.
 * @{
 */

/**
 * Create a new LALSimInspiralWaveformFlags struct
 * with all flags set to their default values.
 *
 * Remember to destroy the struct when you are done with it.
 */
LALSimInspiralWaveformFlags *XLALSimInspiralCreateWaveformFlags(void)
{
    LALSimInspiralWaveformFlags *waveFlags;
    /* Allocate memory for the waveform flags */
    waveFlags = XLALMalloc( sizeof(*waveFlags) );
    if( !waveFlags )
    {
        XLALFree(waveFlags);
        XLAL_ERROR_NULL(XLAL_ENOMEM);
    }

    /* Set all flags to their default values */
    XLALSimInspiralSetSpinOrder(waveFlags,
            LAL_SIM_INSPIRAL_SPIN_ORDER_DEFAULT);
    XLALSimInspiralSetTidalOrder(waveFlags,
            LAL_SIM_INSPIRAL_TIDAL_ORDER_DEFAULT);
    XLALSimInspiralSetFrameAxis(waveFlags,
            LAL_SIM_INSPIRAL_FRAME_AXIS_DEFAULT);
    XLALSimInspiralSetModesChoice(waveFlags,
            LAL_SIM_INSPIRAL_MODES_CHOICE_DEFAULT);

    return waveFlags;
}

/**
 * Destroy a LALSimInspiralWaveformFlags struct.
 */
void XLALSimInspiralDestroyWaveformFlags(
        LALSimInspiralWaveformFlags *waveFlags
        )
{
    if( waveFlags )
        XLALFree(waveFlags);
    return;
}

/**
 * Returns true if waveFlags is non-NULL and all of its fields have default
 * value; returns false otherwise.
 */
bool XLALSimInspiralWaveformParamsFlagsAreDefault(LALDict *params)
{
    /* Check every field of WaveformFlags, each returns 1/0 for true/false.
     * Return true iff waveFlags is non-NULL and all checks are true. */
  return ( XLALSimInspiralWaveformParamsPNSpinOrderIsDefault(params) &&
	   XLALSimInspiralWaveformParamsPNTidalOrderIsDefault(params) &&
	   XLALSimInspiralWaveformParamsFrameAxisIsDefault(params) &&
	   XLALSimInspiralWaveformParamsModesChoiceIsDefault(params));
}

/**
 * Returns true if waveFlags is non-NULL and all of its fields have default
 * value; returns false otherwise.
 */
bool XLALSimInspiralWaveformFlagsIsDefaultOLD(
        LALSimInspiralWaveformFlags *waveFlags
        )
{
    /* Check every field of WaveformFlags, each returns 1/0 for true/false.
     * Return true iff waveFlags is non-NULL and all checks are true. */
    return !waveFlags || (
        XLALSimInspiralSpinOrderIsDefault(waveFlags->spinO) &&
        XLALSimInspiralTidalOrderIsDefault(waveFlags->tideO) &&
        XLALSimInspiralFrameAxisIsDefault(waveFlags->axisChoice) &&
        XLALSimInspiralModesChoiceIsDefault(waveFlags->modesChoice));
}

/**
 * Checks if all flags in two LALSimInspiralWaveformFlags structs are equal.
 * Returns true if all flags are equal. Returns false if one or more differ.
 */
bool XLALSimInspiralWaveformFlagsEqualOLD(
        LALSimInspiralWaveformFlags *waveFlags1,
        LALSimInspiralWaveformFlags *waveFlags2
        )
{
    LALSimInspiralSpinOrder spinO1, spinO2;
    LALSimInspiralTidalOrder tideO1, tideO2;
    LALSimInspiralFrameAxis axisChoice1, axisChoice2;
    LALSimInspiralModesChoice modesChoice1, modesChoice2;

    spinO1 = XLALSimInspiralGetSpinOrder(waveFlags1);
    spinO2 = XLALSimInspiralGetSpinOrder(waveFlags2);
    tideO1 = XLALSimInspiralGetTidalOrder(waveFlags1);
    tideO2 = XLALSimInspiralGetTidalOrder(waveFlags2);
    axisChoice1 = XLALSimInspiralGetFrameAxis(waveFlags1);
    axisChoice2 = XLALSimInspiralGetFrameAxis(waveFlags2);
    modesChoice1 = XLALSimInspiralGetModesChoice(waveFlags1);
    modesChoice2 = XLALSimInspiralGetModesChoice(waveFlags2);

    return ( (spinO1==spinO2) && (tideO1==tideO2) && (axisChoice1==axisChoice2)
            && (modesChoice1==modesChoice2) );
}

/**
 * Checks if all flags in two LALSimInspiralWaveformFlags structs are equal.
 * Returns true if all flags are equal. Returns false if one or more differ.
 */
bool XLALSimInspiralWaveformFlagsEqual(
        LALDict *LALpars1,
        LALDict *LALpars2
        )
{
    LALSimInspiralSpinOrder spinO1, spinO2;
    LALSimInspiralTidalOrder tideO1, tideO2;
    LALSimInspiralFrameAxis axisChoice1, axisChoice2;
    LALSimInspiralModesChoice modesChoice1, modesChoice2;

    spinO1 = XLALSimInspiralWaveformParamsLookupPNSpinOrder(LALpars1);
    spinO2 = XLALSimInspiralWaveformParamsLookupPNSpinOrder(LALpars2);
    tideO1 = XLALSimInspiralWaveformParamsLookupPNTidalOrder(LALpars1);
    tideO2 = XLALSimInspiralWaveformParamsLookupPNTidalOrder(LALpars2);
    axisChoice1 = XLALSimInspiralWaveformParamsLookupFrameAxis(LALpars1);
    axisChoice2 = XLALSimInspiralWaveformParamsLookupFrameAxis(LALpars2);
    modesChoice1 = XLALSimInspiralWaveformParamsLookupModesChoice(LALpars1);
    modesChoice2 = XLALSimInspiralWaveformParamsLookupModesChoice(LALpars2);

    return ( (spinO1==spinO2) && (tideO1==tideO2) && (axisChoice1==axisChoice2)
            && (modesChoice1==modesChoice2) );
}

/**
 * Set the LALSimInspiralSpinOrder within a LALSimInspiralWaveformFlags struct
 */
void XLALSimInspiralSetSpinOrder(
        LALSimInspiralWaveformFlags *waveFlags, /**< Struct whose flag will be set */

        LALSimInspiralSpinOrder spinO /**< value to set flag to */
        )
{
    waveFlags->spinO = spinO;
    return;
}

/**
 * Get the LALSimInspiralSpinOrder within a LALSimInspiralWaveformFlags struct,
 * or LAL_SIM_INSPIRAL_SPIN_ORDER_DEFAULT if waveFlags is NULL
 */
LALSimInspiralSpinOrder XLALSimInspiralGetSpinOrder(
        LALSimInspiralWaveformFlags *waveFlags
        )
{
    if ( waveFlags )
        return waveFlags->spinO;
    else
        return LAL_SIM_INSPIRAL_SPIN_ORDER_DEFAULT;
}

/**
 * Returns true if LALSimInspiralSpinOrder has default value
 * returns false otherwise
 */
bool XLALSimInspiralSpinOrderIsDefault(
        LALSimInspiralSpinOrder spinO
        )
{
    if( spinO == LAL_SIM_INSPIRAL_SPIN_ORDER_DEFAULT )
        return true;
    else
        return false;
}

/**
 * Set the LALSimInspiralTidalOrder within a LALSimInspiralWaveformFlags struct
 */
void XLALSimInspiralSetTidalOrder(
        LALSimInspiralWaveformFlags *waveFlags, /**< Struct whose flag will be set */

        LALSimInspiralTidalOrder tideO /**< value to set flag to */
        )
{
    waveFlags->tideO = tideO;
    return;
}

/**
 * Get the LALSimInspiralTidalOrder within a LALSimInspiralWaveformFlags struct,
 * or LAL_SIM_INSPIRAL_TIDAL_ORDER_DEFAULT if waveFlags is NULL
 */
LALSimInspiralTidalOrder XLALSimInspiralGetTidalOrder(
        LALSimInspiralWaveformFlags *waveFlags
        )
{
    if ( waveFlags )
        return waveFlags->tideO;
    else
        return LAL_SIM_INSPIRAL_TIDAL_ORDER_DEFAULT;
}

/**
 * Returns true if LALSimInspiralTidalOrder has default value
 * returns false otherwise
 */
bool XLALSimInspiralTidalOrderIsDefault(
        LALSimInspiralTidalOrder tideO
        )
{
    if( tideO == LAL_SIM_INSPIRAL_TIDAL_ORDER_DEFAULT )
        return true;
    else
        return false;
}

/**
 * Set the LALSimInspiralFrameAxis within a LALSimInspiralWaveformFlags struct
 */
void XLALSimInspiralSetFrameAxis(
        LALSimInspiralWaveformFlags *waveFlags, /**< Struct whose flag will be set */
        LALSimInspiralFrameAxis axisChoice /**< value to set flag to */
        )
{
    waveFlags->axisChoice = axisChoice;
    return;
}

/**
 * Get the LALSimInspiralFrameAxis within a LALSimInspiralWaveformFlags struct,
 * or LAL_SIM_INSPIRAL_FRAME_AXIS_DEFAULT if waveFlags is NULL
 */
LALSimInspiralFrameAxis XLALSimInspiralGetFrameAxis(
        LALSimInspiralWaveformFlags *waveFlags
        )
{
    if ( waveFlags )
        return waveFlags->axisChoice;
    else
        return LAL_SIM_INSPIRAL_FRAME_AXIS_DEFAULT;
}

/**
 * Returns true if LALSimInspiralFrameAxis has default value
 * returns false otherwise
 */
bool XLALSimInspiralFrameAxisIsDefault(
        LALSimInspiralFrameAxis axisChoice
        )
{
    if( axisChoice == LAL_SIM_INSPIRAL_FRAME_AXIS_DEFAULT )
        return true;
    else
        return false;
}

/**
 * Set the LALSimInspiralModesChoice within a LALSimInspiralWaveformFlags struct
 */
void XLALSimInspiralSetModesChoice(
        LALSimInspiralWaveformFlags *waveFlags, /**< Struct whose flag will be set */
        LALSimInspiralModesChoice modesChoice /**< value to set flag to */
        )
{
    waveFlags->modesChoice = modesChoice;
    return;
}

/**
 * Get the LALSimInspiralModesChoice within a LALSimInspiralWaveformFlags struct,
 * or LAL_SIM_INSPIRAL_MODES_CHOICE_DEFAULT if waveFlags is NULL
 */
LALSimInspiralModesChoice XLALSimInspiralGetModesChoice(
        LALSimInspiralWaveformFlags *waveFlags
        )
{
    if ( waveFlags )
        return waveFlags->modesChoice;
    else
        return LAL_SIM_INSPIRAL_MODES_CHOICE_DEFAULT;
}

/**
 * Returns true if LALSimInspiralModesChoice has default value
 * returns false otherwise
 */
bool XLALSimInspiralModesChoiceIsDefault(
        LALSimInspiralModesChoice modesChoice
        )
{
    if( modesChoice == LAL_SIM_INSPIRAL_MODES_CHOICE_DEFAULT )
        return true;
    else
        return false;
}

/**
 * Set the numreldata string within a LALSimInspiralWaveformFlags struct
 */
void XLALSimInspiralSetNumrelDataOLD(
        LALSimInspiralWaveformFlags *waveFlags, /**< Struct whose value will be set */
        const char* numreldata /**< value to set numreldata to */
        )
{
    XLALStringCopy(waveFlags->numreldata, numreldata, sizeof(waveFlags->numreldata));
    return;
}

/**
 * Returns a deepcopy of the pointer of the numeraldata attribute of the
 * waveFlags structure. If this is NULL then NULL will be returned.
 * The returned value is independent of the waveFlags structure and will
 * need to be LALFree-d.
 */
char* XLALSimInspiralGetNumrelDataOLD(
        LALSimInspiralWaveformFlags *waveFlags
        )
{
    char *ret_string;
    if ( waveFlags )
    {
        ret_string = XLALMalloc(FILENAME_MAX * sizeof(char));
        XLALStringCopy(ret_string, waveFlags->numreldata, sizeof(waveFlags->numreldata));
        return ret_string;
    }
    else
    {
        return NULL;
    }
}


/**
 * Returns a INT4VectorSequence pointer.
 * With length = (LAL_SIM_INSPIRAL_LMAX - LAL_SIM_INSPIRAL_LMIN) + 1
 * and vectorLength = 2*LAL_SIM_INSPIRAL_LMAX + 1.
 * length is the total number of ell spherical harmonic modes
 * vectorLength is the largest number of m modes of the largest ell mode
 * considered.
 * The largest ell mode is given by LAL_SIM_INSPIRAL_LMAX.
 * This returned 'ModeArray' is used in lalsimulation to
 * identify which spherical harmonic multipoles should be used
 * when generating a waveform.
 * The following convention is adopted for the entires of ModeArray
 * value = 1 means mode on
 * value = 0 means mode off
 * value = -1 means 'm' mode does not exist for this ell mode.
 * This function returns a 'ModeArray' filled with 0's for valid
 * entries and with -1's for invalid/unphysical entires.
 */
INT4VectorSequence * XLALSimInspiralCreateModeArray(void)
{
     /* num_l_modes: counts the number spherical harmonic ell modes from LAL_SIM_INSPIRAL_LMIN to LAL_SIM_INSPIRAL_LMAX (inclusive)*/
    int num_l_modes = (LAL_SIM_INSPIRAL_LMAX - LAL_SIM_INSPIRAL_LMIN) + 1;
    /* max_num_m_modes: the largest number of spherical harmonics m modes of the highest ell mode (LAL_SIM_INSPIRAL_LMAX) */
    int max_num_m_modes = 2*LAL_SIM_INSPIRAL_LMAX + 1;
    /* initialize ModeArray */
    INT4VectorSequence* ModeArray = XLALCreateINT4VectorSequence(num_l_modes, max_num_m_modes);

    /*  fill array with zeros for valid entry points */
    /*  and with -1 for invalid points */

     /* initialize */
    int ell = 0;
    int m = 0;
    int one_dim_array_index = 0;
    /* loop over all ModeArray entries */
    for (int ell_index=0; ell_index<num_l_modes; ell_index++){
        ell = ell_index + 2; /* because the lowest ell mode we consider is ell=2 and the array index starts counting from 0 we convert from the ell array index to actual ell. */
        for (int m_mode_index=0; m_mode_index<max_num_m_modes; m_mode_index++){
            m = XLALSimInspiralModeArrayConvertIndexToM(ell, m_mode_index);
            one_dim_array_index = XLALSimInspiralModeArrayConvertModeToIndex(ell, m); /* Convert to 1D array index */
            /* if m mode is unphsical, i.e. > 2*ell + 1 then set it to -1. Else set it to 0. */
            if (m_mode_index >= XLALSimInspiralModeArrayLastIndexForL(ell)) {
                ModeArray->data[one_dim_array_index] = -1;
            } else {
                ModeArray->data[one_dim_array_index] = 0;
            }
        }
    }
    return ModeArray;
}


/**
 * Input:
 * ell: spherical harmonic ell mode
 * m: spherical harmonic m mode
 * into the one-dimensional array index.
 */
int XLALSimInspiralModeArrayConvertModeToIndex(int ell, int m)
{
    int ell_index = ell - 2;
    /* max_num_m_modes: the largest number of spherical harmonics m modes of the highest ell mode (LAL_SIM_INSPIRAL_LMAX) */
    int max_num_m_modes = 2*LAL_SIM_INSPIRAL_LMAX + 1;

    int m_index = XLALSimInspiralModeArrayConvertMToIndex(ell, m);

    return ell_index*max_num_m_modes + m_index;
}

/**
 * Converts the m spherical harmonic to it's corresponding array index.
 */
int XLALSimInspiralModeArrayConvertMToIndex(int ell, int m)
{
    return ell + m;
}

/**
 * Converts the m spherical harmonic array index to it's corresponding physical m-mode value.
 */
int XLALSimInspiralModeArrayConvertIndexToM(int ell, int index)
{
    return index - ell;
}

/**
 * Returns the array index of the highest m-mode
 * for a given ell spherical harmonic.
 * The last index is (2*ell + 1).
 * This is used to avoid unessessary looping over
 * m modes that are unphysical.
 */
int XLALSimInspiralModeArrayLastIndexForL(int ell)
{
    return 2*ell + 1;
}

/**
 * Check if the given ell, m spherical harmonic mode is active in ModeArray.
 * Returns 1 if active
 * Returns 0 if inactive.
 */
int XLALSimInspiralModeArrayIsModeActive(INT4VectorSequence *ModeArray, int ell, int m)
{
    int ret = 0;
    int one_dim_array_index = XLALSimInspiralModeArrayConvertModeToIndex(ell, m); /* Convert to 1D array index */
    if (ModeArray->data[one_dim_array_index] == 1) {
        ret = 1;
    }

    return ret;
}

/**
 * add all m modes at given ell to ModeArray
 */
int XLALSimInspiralModeArrayAddAllModesAtL(INT4VectorSequence *ModeArray, int ell)
{

    XLAL_CHECK(!(ell>8), XLAL_EDOM, "ell index is out of range. Currently only ell <= %i supported\n", LAL_SIM_INSPIRAL_LMAX);
    XLAL_CHECK(!(ell<2), XLAL_EDOM, "ell index is out of range. ell must start at %i\n", LAL_SIM_INSPIRAL_LMIN);

    int one_dim_array_index = 0;

    //to add all at input ell
    for (int m=-ell; m<=ell; m++){
        one_dim_array_index = XLALSimInspiralModeArrayConvertModeToIndex(ell, m); /* Convert to 1D array index */
        ModeArray->data[one_dim_array_index] = 1;
    }

    return XLAL_SUCCESS;
}

/**
 * remove all m modes at given ell from ModeArray
 */
int XLALSimInspiralModeArrayRemoveAllModesAtL(INT4VectorSequence *ModeArray, int ell)
{

    XLAL_CHECK(!(ell>8), XLAL_EDOM, "ell index is out of range. Currently only ell <= %i supported\n", LAL_SIM_INSPIRAL_LMAX);
    XLAL_CHECK(!(ell<2), XLAL_EDOM, "ell index is out of range. ell must start at %i\n", LAL_SIM_INSPIRAL_LMIN);

    int one_dim_array_index = 0;

    //to add all at input ell
    for (int m=-ell; m<=ell; m++){
        one_dim_array_index = XLALSimInspiralModeArrayConvertModeToIndex(ell, m); /* Convert to 1D array index */
        ModeArray->data[one_dim_array_index] = 0;
    }

    return XLAL_SUCCESS;
}

/**
 * add ell and m mode to ModeArray
 */
int XLALSimInspiralModeArrayAddMode(INT4VectorSequence *ModeArray, int ell, int m)
{

    XLAL_CHECK(!(ell>8), XLAL_EDOM, "ell index is out of range. Currently only ell <= %i supported\n", LAL_SIM_INSPIRAL_LMAX);
    XLAL_CHECK(!(ell<2), XLAL_EDOM, "ell index is out of range. ell must start at %i\n", LAL_SIM_INSPIRAL_LMIN);
    XLAL_CHECK(!(m>ell), XLAL_EDOM, "m index is out of range. m index must run from -ell to ell\n");
    XLAL_CHECK(!(m<-ell), XLAL_EDOM, "m index is out of range. m index must run from -ell to ell\n");

    int one_dim_array_index = XLALSimInspiralModeArrayConvertModeToIndex(ell, m); /* Convert to 1D array index */

    ModeArray->data[one_dim_array_index] = 1;
    return XLAL_SUCCESS;
}

/**
 * remove ell and m mode from ModeArray
 */
int XLALSimInspiralModeArrayRemoveMode(INT4VectorSequence *ModeArray, int ell, int m)
{
    XLAL_CHECK(!(ell>8), XLAL_EDOM, "ell index is out of range. Currently only ell <= %i supported\n", LAL_SIM_INSPIRAL_LMAX);
    XLAL_CHECK(!(ell<2), XLAL_EDOM, "ell index is out of range. ell must start at %i\n", LAL_SIM_INSPIRAL_LMIN);
    XLAL_CHECK(!(m>ell), XLAL_EDOM, "m index is out of range. m index must run from -ell to ell\n");
    XLAL_CHECK(!(m<-ell), XLAL_EDOM, "m index is out of range. m index must run from -ell to ell\n");

    int one_dim_array_index = XLALSimInspiralModeArrayConvertModeToIndex(ell, m); /* Convert to 1D array index */

    ModeArray->data[one_dim_array_index] = 0;
    return XLAL_SUCCESS;
}

/**
 * Use a set of predetermined options to add gravitational wave
 * spin weighted spherical modes to ModeArray.
 * Update me with more options.
 * The LALSimulation struct LALSimInspiralModesChoice is used.
 */
int XLALSimInspiralModeArraySetupPresets(INT4VectorSequence *ModeArray, LALSimInspiralModesChoice ModesChoice)
{

    switch ( ModesChoice ) {
        case LAL_SIM_INSPIRAL_MODES_CHOICE_RESTRICTED: //2-2 and 22 (also called LAL_SIM_INSPIRAL_MODES_CHOICE_DEFAULT)
            //NOTE: To use this in python you have to call it LAL_SIM_INSPIRAL_MODES_CHOICE_RESTRICTED ... Don't know why
            XLALSimInspiralModeArrayAddMode(ModeArray, 2,-2);
            XLALSimInspiralModeArrayAddMode(ModeArray, 2,2);
            break;
        case LAL_SIM_INSPIRAL_MODES_CHOICE_ALL:
            for (int ell=2; ell<=LAL_SIM_INSPIRAL_LMAX; ell++){
                XLALSimInspiralModeArrayAddAllModesAtL(ModeArray, ell);
            }
            break;
        default:
            XLALPrintError("Unknown value for ModesChoice\n");
            XLAL_ERROR(XLAL_EINVAL);
    }

    return XLAL_SUCCESS;
}

/** @} */
