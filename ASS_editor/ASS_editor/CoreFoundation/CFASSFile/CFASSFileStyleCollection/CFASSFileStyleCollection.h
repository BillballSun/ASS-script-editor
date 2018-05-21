//
//  CFASSFileStyleCollection.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileStyleCollection_h
#define CFASSFileStyleCollection_h

typedef struct CFASSFileStyleCollection *CFASSFileStyleCollectionRef;

CFASSFileStyleCollectionRef CFASSFileStyleCollectionCopy(CFASSFileStyleCollectionRef styleCollection);

void CFASSFileStyleCollectionDestory(CFASSFileStyleCollectionRef styleCollection);

#endif /* CFASSFileStyleCollection_h */
