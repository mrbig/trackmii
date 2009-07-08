/* 
 * File:   trackmii_plugin.h
 * Author: mrbig
 *
 * Created on 2009. j�lius 6., 20:47
 */

#ifndef _TRACKMII_PLUGIN_H
#define	_TRACKMII_PLUGIN_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct basicTranslationCfg {
    int deadzone;
    int response;
    int amplification;
} basicTranslationCfg;

basicTranslationCfg getTranslationCfg(int dof);
void setTranslationCfg(int dof, basicTranslationCfg* cfg);

void ConnectWiimote();

int getConnectionState();

#ifdef	__cplusplus
}
#endif

#endif	/* _TRACKMII_PLUGIN_H */

