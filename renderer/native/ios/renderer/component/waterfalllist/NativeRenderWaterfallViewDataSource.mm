/*!
 * iOS SDK
 *
 * Tencent is pleased to support the open source community by making
 * Hippy available.
 *
 * Copyright (C) 2019 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import <UIKit/NSIndexPath+UIKitAdditions.h>

#import "HippyAssert.h"
#import "NativeRenderWaterfallViewDataSource.h"
#import "HippyShadowView.h"
#import "HippyShadowListView.h"

@interface NativeRenderWaterfallViewDataSource () {
    BOOL _containBannerView;
    NSArray<NSArray<HippyShadowView *> *> *_cellRenderObjectViews;
    HippyShadowView *_bannerView;
}

@end

@implementation NativeRenderWaterfallViewDataSource

- (instancetype)initWithDataSource:(NSArray<__kindof HippyShadowView *> *)dataSource
                      itemViewName:(NSString *)itemViewName
                 containBannerView:(BOOL)containBannerView {
    self = [super init];
    if (self) {
        self.itemViewName = itemViewName;
        [self setDataSource:dataSource containBannerView:containBannerView];
    }
    return self;
}

- (id)copyWithZone:(nullable NSZone *)zone {
    NativeRenderWaterfallViewDataSource *dataSource = [[[self class] allocWithZone:zone] init];
    dataSource->_containBannerView = self.containBannerView;
    dataSource->_bannerView = _bannerView;
    NSMutableArray<NSArray<HippyShadowView *> *> *objectSectionViews = [NSMutableArray arrayWithCapacity:[_cellRenderObjectViews count]];
    for (NSArray<HippyShadowView *> *objects in _cellRenderObjectViews) {
        NSArray<HippyShadowView *> *copiedObjects = [objects copy];
        [objectSectionViews addObject:copiedObjects];
    }
    dataSource->_cellRenderObjectViews = [objectSectionViews copy];
    dataSource.itemViewName = [self.itemViewName copy];
    return dataSource;
}

- (void)setDataSource:(NSArray<HippyShadowView *> *)dataSource {
    [self setDataSource:dataSource containBannerView:NO];
}

- (void)setDataSource:(NSArray<HippyShadowView *> *)dataSource
    containBannerView:(BOOL)containBannerView {
    _containBannerView = containBannerView;
    if ([dataSource count] > 0) {
        if (containBannerView) {
            _bannerView = [dataSource firstObject];
        }
        NSUInteger loc = _containBannerView ? 1 : 0;
        NSArray<HippyShadowView *> *candidateRenderObjectViews = [dataSource subarrayWithRange:NSMakeRange(loc, [dataSource count] - loc)];
        NSString *viewName = self.itemViewName;
        static dispatch_once_t onceToken;
        static NSPredicate *prediate = nil;
        dispatch_once(&onceToken, ^{
            prediate = [NSPredicate predicateWithBlock:^BOOL(id  _Nullable evaluatedObject, NSDictionary<NSString *,id> * _Nullable bindings) {
                HippyShadowView *renderObjectView = (HippyShadowView *)evaluatedObject;
                if ([renderObjectView.viewName isEqualToString:viewName]) {
                    return YES;
                }
                return NO;
            }];
        });
        NSArray<HippyShadowView *> *objects = [candidateRenderObjectViews filteredArrayUsingPredicate:prediate];
        if ([objects count]) {
            _cellRenderObjectViews = [NSArray arrayWithObject:objects];
        }
        else {
            _cellRenderObjectViews = nil;
        }
    }
}

-(HippyShadowView *)bannerView {
    return _bannerView;
}

- (NSArray<NSArray<HippyShadowView *> *> *)cellRenderObjectViews {
    return [_cellRenderObjectViews copy];
}

- (HippyShadowView *)cellForIndexPath:(NSIndexPath *)indexPath {
    if (_containBannerView && 0 == [indexPath section]) {
        return _bannerView;
    }
    else {
        return [[_cellRenderObjectViews firstObject] objectAtIndex:[indexPath row]];
    }
}

- (HippyShadowView *)headerForSection:(NSInteger)section {
    return nil;
}

- (NSInteger)numberOfSection {
    NSInteger count = _containBannerView ? 1  : 0;
    count += [[_cellRenderObjectViews firstObject] count] ? 1 : 0;
    return count;
}

- (NSInteger)numberOfCellForSection:(NSInteger)section {
    if (_containBannerView) {
        return 0 == section ? 1 : [[_cellRenderObjectViews firstObject] count];
    }
    else {
        return [[_cellRenderObjectViews firstObject] count];
    }
}

- (NSIndexPath *)indexPathOfCell:(HippyShadowView *)cell {
    NSInteger row = 0;
    NSInteger section = 0;
    if (_containBannerView) {
        if (_bannerView != cell) {
            section = 1;
            row = [[_cellRenderObjectViews firstObject] indexOfObject:cell];
        }
    }
    else {
        row = [[_cellRenderObjectViews firstObject] indexOfObject:cell];
    }
    return [NSIndexPath indexPathForRow:row inSection:section];
}

- (NSIndexPath *)indexPathForFlatIndex:(NSInteger)index {
    NSInteger row = 0;
    NSInteger section = 0;
    if (_containBannerView) {
        if (0 != index) {
            section = 1;
            index -= 1;
        }
    }
    else {
        row = index;
    }
    return [NSIndexPath indexPathForRow:row inSection:section];
}

- (NSInteger)flatIndexForIndexPath:(NSIndexPath *)indexPath {
    NSInteger row = [indexPath row];
    NSInteger section = [indexPath section];
    NSInteger index = 0;
    if (_containBannerView) {
        if (0 != section) {
            index = row + 1;
        }
    }
    else {
        index = row;
    }
    return index;
}


@end
