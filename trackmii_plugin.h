/* Copyright (C) 2009 Nagy Attila Gabor <nagy.attila.gabor@gmail.com>
 *
 *     This file is part of TrackMii.
 *
 *  TrackMii is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TrackMii is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _TRACKMII_PLUGIN_H
#define	_TRACKMII_PLUGIN_H

#ifdef	__cplusplus
extern "C" {
#endif

#define TRACKMII_VERSION 1
// Number of cycles between checking wiimote state
#define STATE_CHECK_INTERVAL 150;

typedef struct basicTranslationCfg {
    int deadzone;
    int response;
    int amplification;
} basicTranslationCfg;

basicTranslationCfg getTranslationCfg(int dof);
void setTranslationCfg(int dof, basicTranslationCfg* cfg);

void ConnectWiimote();

int getConnectionState();

void SaveSettings();

void ToggleDebugWindowVisible(int state);

#ifdef	__cplusplus
}
#endif

#endif	/* _TRACKMII_PLUGIN_H */

