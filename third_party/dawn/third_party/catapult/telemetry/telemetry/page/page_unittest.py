# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import unittest
from unittest import mock

from telemetry import story
from telemetry.page import page


class TestPage(unittest.TestCase):

  def assertPathEqual(self, path1, path2):
    self.assertEqual(os.path.normpath(path1), os.path.normpath(path2))

  def testFilePathRelative(self):
    apage = page.Page('file://somedir/otherdir/file.html',
                      None, base_dir='basedir', name='test')
    self.assertPathEqual(apage.file_path, 'basedir/somedir/otherdir/file.html')

  def testFilePathAbsolute(self):
    apage = page.Page('file:///somedir/otherdir/file.html',
                      None, base_dir='basedir', name='test')
    self.assertPathEqual(apage.file_path, '/somedir/otherdir/file.html')

  def testFilePathQueryString(self):
    apage = page.Page('file://somedir/otherdir/file.html?key=val',
                      None, base_dir='basedir', name='test')
    self.assertPathEqual(apage.file_path, 'basedir/somedir/otherdir/file.html')

  def testFilePathUrlQueryString(self):
    apage = page.Page('file://somedir/file.html?key=val',
                      None, base_dir='basedir', name='test')
    self.assertPathEqual(apage.file_path_url,
                         'basedir/somedir/file.html?key=val')

  def testFilePathUrlTrailingSeparator(self):
    apage = page.Page('file://somedir/otherdir/',
                      None, base_dir='basedir', name='test')
    self.assertPathEqual(apage.file_path_url, 'basedir/somedir/otherdir/')
    self.assertTrue(apage.file_path_url.endswith(os.sep) or
                    (os.altsep and apage.file_path_url.endswith(os.altsep)))

  def testSort(self):
    story_set = story.StorySet(base_dir=os.path.dirname(__file__))
    story_set.AddStory(
        page.Page('http://www.foo.com/', story_set, story_set.base_dir,
                  name='foo'))
    story_set.AddStory(
        page.Page('http://www.bar.com/', story_set, story_set.base_dir,
                  name='bar'))

    pages = sorted([story_set.stories[0], story_set.stories[1]])
    self.assertEqual([story_set.stories[1], story_set.stories[0]], pages)

  def testGetUrlBaseDirAndFileForUrlBaseDir(self):
    base_dir = os.path.dirname(__file__)
    file_path = os.path.join(
        os.path.dirname(base_dir), 'otherdir', 'file.html')
    story_set = story.StorySet(base_dir=base_dir,
                               serving_dirs=[os.path.join('..', 'somedir', '')])
    story_set.AddStory(
        page.Page('file://../otherdir/file.html', story_set,
                  story_set.base_dir, name='test'))
    self.assertPathEqual(story_set[0].file_path, file_path)

  def testDisplayUrlForHttp(self):
    story_set = story.StorySet(base_dir=os.path.dirname(__file__))
    story_set.AddStory(
        page.Page('http://www.foo.com/', story_set, story_set.base_dir,
                  name='http://www.foo.com/'))
    story_set.AddStory(
        page.Page('http://www.bar.com/', story_set, story_set.base_dir,
                  name='http://www.bar.com/'))

    self.assertEqual(story_set[0].name, 'http://www.foo.com/')
    self.assertEqual(story_set[1].name, 'http://www.bar.com/')

  def testDisplayUrlForHttps(self):
    story_set = story.StorySet(base_dir=os.path.dirname(__file__))
    story_set.AddStory(
        page.Page('http://www.foo.com/', story_set, story_set.base_dir,
                  name='http://www.foo.com/'))
    story_set.AddStory(
        page.Page('https://www.bar.com/', story_set, story_set.base_dir,
                  name='https://www.bar.com/'))

    self.assertEqual(story_set[0].name, 'http://www.foo.com/')
    self.assertEqual(story_set[1].name, 'https://www.bar.com/')

  def testDisplayUrlForFile(self):
    story_set = story.StorySet(base_dir=os.path.dirname(__file__))
    story_set.AddStory(page.Page(
        'file://../../otherdir/foo.html', story_set, story_set.base_dir,
        name='foo.html'))
    story_set.AddStory(page.Page(
        'file://../../otherdir/bar.html', story_set, story_set.base_dir,
        name='bar.html'))

    self.assertEqual(story_set[0].name, 'foo.html')
    self.assertEqual(story_set[1].name, 'bar.html')

  def testDisplayUrlForFilesDifferingBySuffix(self):
    story_set = story.StorySet(base_dir=os.path.dirname(__file__))
    story_set.AddStory(page.Page(
        'file://../../otherdir/foo.html', story_set, story_set.base_dir,
        name='foo.html'))
    story_set.AddStory(page.Page(
        'file://../../otherdir/foo1.html', story_set, story_set.base_dir,
        name='foo1.html'))

    self.assertEqual(story_set[0].name, 'foo.html')
    self.assertEqual(story_set[1].name, 'foo1.html')

  def testDisplayUrlForFileOfDifferentPaths(self):
    story_set = story.StorySet(base_dir=os.path.dirname(__file__))
    story_set.AddStory(
        page.Page(
            'file://../../somedir/foo.html', story_set, story_set.base_dir,
            name='somedir/foo.html'))
    story_set.AddStory(page.Page(
        'file://../../otherdir/bar.html', story_set, story_set.base_dir,
        name='otherdir/bar.html'))

    self.assertEqual(story_set[0].name, 'somedir/foo.html')
    self.assertEqual(story_set[1].name, 'otherdir/bar.html')

  def testDisplayUrlForFileDirectories(self):
    story_set = story.StorySet(base_dir=os.path.dirname(__file__))
    story_set.AddStory(
        page.Page('file://../../otherdir/foo', story_set, story_set.base_dir,
                  name='foo'))
    story_set.AddStory(
        page.Page('file://../../otherdir/bar', story_set, story_set.base_dir,
                  name='bar'))

    self.assertEqual(story_set[0].name, 'foo')
    self.assertEqual(story_set[1].name, 'bar')

  def testDisplayUrlForSingleFile(self):
    story_set = story.StorySet(base_dir=os.path.dirname(__file__))
    story_set.AddStory(page.Page(
        'file://../../otherdir/foo.html', story_set, story_set.base_dir,
        name='foo.html'))

    self.assertEqual(story_set[0].name, 'foo.html')

  def testDisplayUrlForSingleDirectory(self):
    story_set = story.StorySet(base_dir=os.path.dirname(__file__))
    story_set.AddStory(
        page.Page('file://../../otherdir/foo', story_set, story_set.base_dir,
                  name='foo'))

    self.assertEqual(story_set[0].name, 'foo')

  def testPagesHaveDifferentIds(self):
    p0 = page.Page("http://example.com", name='example1')
    p1 = page.Page("http://example.com", name='example2')
    self.assertNotEqual(p0.id, p1.id)

  def testNamedPageAsDict(self):
    named_dict = page.Page('http://example.com/', name='Example').AsDict()
    self.assertIn('id', named_dict)
    del named_dict['id']
    self.assertEqual({
        'url': 'http://example.com/',
        'name': 'Example'
    }, named_dict)

  def testIsLocal(self):
    p = page.Page('file://foo.html', name='foo.html')
    self.assertTrue(p.is_local)

    p = page.Page('chrome://extensions', name='extensions')
    self.assertTrue(p.is_local)

    p = page.Page('about:blank', name='about:blank')
    self.assertTrue(p.is_local)

    p = page.Page('http://foo.com', name='http://foo.com')
    self.assertFalse(p.is_local)


class TestPageRun(unittest.TestCase):

  def testFiveGarbageCollectionCallsByDefault(self):
    mock_shared_state = mock.MagicMock()
    mock_action_runner = mock.MagicMock()
    with mock.patch('telemetry.internal.actions.action_runner.ActionRunner',
                    new=lambda current_tab, skip_waits: mock_action_runner):
      p = page.Page('file://foo.html', name='foo.html')
      p.Run(mock_shared_state)
      expected = [
          mock.call.current_tab.CollectGarbage(),
          mock.call.current_tab.CollectGarbage(),
          mock.call.current_tab.CollectGarbage(),
          mock.call.current_tab.CollectGarbage(),
          mock.call.current_tab.CollectGarbage(),
          mock.call.interval_profiling_controller.SamplePeriod(
              'story_run', mock_action_runner),
          mock.call.interval_profiling_controller.SamplePeriod().__enter__(),
          mock.call.NavigateToPage(mock_action_runner, p),
          mock.call.RunPageInteractions(mock_action_runner, p),
          mock.call.interval_profiling_controller.SamplePeriod().__exit__(
              None, None, None)
      ]
      self.assertEqual(mock_shared_state.mock_calls, expected)
