//
//  ViewController.h
//  Vowels
//
//  Created by Pedro Pinto on 17/06/13.
//  Copyright (c) 2013 Pedro Pinto. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "FormantView.h"

@interface ViewController : UIViewController

@property (nonatomic, retain) IBOutlet UIImageView *background;
@property (nonatomic, retain) IBOutlet FormantView *canvas;

@end
