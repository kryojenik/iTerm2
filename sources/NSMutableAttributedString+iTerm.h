//
//  NSMutableAttributedString+iTerm.h
//  iTerm
//
//  Created by George Nachman on 12/8/13.
//
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

extern NSAttributedStringKey iTermReplacementBaseCharacterAttributeName;

@interface NSMutableAttributedString (iTerm)

- (void)iterm_appendString:(NSString *)string;
- (void)iterm_appendString:(NSString *)string withAttributes:(NSDictionary *)attributes;
- (void)trimTrailingWhitespace;
- (void)replaceAttributesInRange:(NSRange)range withAttributes:(NSDictionary *)newAttributes;

@end

@interface NSAttributedString (iTerm)

+ (instancetype)attributedStringWithAttributedStrings:(NSArray<NSAttributedString *> *)strings;
+ (instancetype)attributedStringWithLinkToURL:(NSString *)urlString string:(NSString *)string;
+ (instancetype)attributedStringWithString:(NSString *)string attributes:(NSDictionary *)attributes;
- (NSArray *)attributedComponentsSeparatedByString:(NSString *)separator;
- (CGFloat)heightForWidth:(CGFloat)maxWidth;
- (NSAttributedString *)attributedStringByRemovingColor;

@end

NS_ASSUME_NONNULL_END
