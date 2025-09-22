/***************************************************************************
 *   Copyright (C) 2005-2007 by Joseph Huang                               *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/* Dev Testing Flags */

#define DEBUGASSERT
// #define NO_LOG_FILES
// ^^^ Enable if you are in a situation where you don't want to pollute the disk on every run
// ^^^ Disable (i.e. explicitly `#undef NO_LOG_FILES`) if you don't want `.github/workflows/ci.yml` to test that it compiles

// #define RTTIASSERT
// ^^^ You can explicitly `#undef RTTIASSERT` if you don't want to allow for Makefile control of these variables

/***********************/

/* Game Flags */
#undef EXTERNAL_DEALER

/******************/

#ifndef NO_LOG_FILES
/* Mode Flags (Depends on NO_LOG_FILES) */
#define LOGPOSITION

/**********************************/
#undef DEBUG_GAIN

#define GRAPHMONEY "chipcount.csv.txt"
#undef DUMP_CSV_PLOTS

/* Savegame debugging */
#undef DEBUG_SINGLE_HAND
#endif




/* AI Processing Interface */

// #define PROGRESSUPDATE 140
// [!TIP]
// You can explicitly `#undef PROGRESSUPDATE`  if you don't want to allow for Makefile control of these variables
#undef SUPERPROGRESSUPDATE
#undef DEBUG_TESTDEALINTERFACE

/***********************/

/* Randomizer Flags */

#undef FORCESEED
#undef COOLSEEDINGVIEWER

/*****************/

#define SEATS_AT_TABLE 10

/* functionbase.h Debugging */
#define DEBUG_TRACE_PWIN
#define DEBUG_TRACE_ZERO
// #define DEBUG_TRACE_SEARCH
// ^^^ Enable if you need to trace through a specific search (usually you'll set `.traceEnable = std::cout` or whatever output stream, near where the issue occurs)
// ^^^ Disable (e.g. You can explicitly `#undef DEBUG_TRACE_SEARCH`) if you don't want `.github/workflows/ci.yml` to test with it
