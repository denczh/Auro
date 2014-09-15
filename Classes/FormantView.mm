//
//  FormantView.m
//  LabVowels
//
//  Created by Pedro Pinto on 26/06/13.
//
//

#import "FormantView.h"
#import "AppDelegate.h"
#import "Types.h"

@implementation FormantView

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
    }
    return self;
}

// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect
{
    // Drawing code
    AppDelegate *appDelegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
    
    double outFormants[2] = {0.0};
    if ([appDelegate getFormants:outFormants]) {
        double x = (480.0/2500.0)*outFormants[F2];
        double y = 320.0 - (320.0/1000.0)*outFormants[F1];
        x = x > 480.0 ? 480.0 : x;
        y = y > 320.0 ? 320.0 : y;
        
        CGContextRef currentContext = UIGraphicsGetCurrentContext();
        [[UIColor blackColor] set];
        CGContextSetLineWidth (currentContext, 5.0);

        // Circle
        CGRect rect = {{x - 30.0, y - 30.0}, {60.0, 60.0}};
        CGContextAddEllipseInRect(currentContext, rect);
       
        CGContextStrokePath(currentContext);

        
        /*
         NSString *string1 = [[NSString alloc] initWithFormat:@"%d", (int)outFormants[F1]];
         NSString *string2 = [[NSString alloc] initWithFormat:@"%d", (int)outFormants[F2]];
         self.viewController.formant1.text = string1;
         self.viewController.formant2.text = string2;
         [string1 release];
         [string2 release];
         */
    }
}

@end
