/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UTILS_PATH_H_
#define UTILS_PATH_H_

#include <utils/compiler.h>

#include <iosfwd>
#include <string>
#include <vector>

namespace utils {

/**
 * An abstract representation of file and directory paths.
 */
class UTILS_PUBLIC Path {
public:
    /**
     * Creates a new empty path.
     */
    Path() = default;
    ~Path() = default;

    /**
     * Creates a new path with the specified pathname.
     *
     * @param pathname a non-null pathname string
     */
    Path(const char* pathname);

    /**
     * Creates a new path with the specified pathname.
     *
     * @param pathname a pathname string
     */
    Path(const std::string& pathname);

    /**
     * Tests whether the file or directory denoted by this abstract
     * pathname exists.
     *
     * @return true if the file or directory denoted by this
     *         abstract pathname exists, false otherwise
     */
    bool exists() const;

    /**
     * Tests whether this abstract pathname represents a regular file.
     * This method can only return true if the path exists.
     *
     * @return true if this pathname represents an existing file,
     *         false if the path doesn't exist or represents something
     *         else (directory, symlink, etc.)
     */
    bool isFile() const;

    /**
     * Tests whether this abstract pathname represents a directory.
     * This method can only return true if the path exists.
     *
     * @return true if this pathname represents an existing directory,
     *         false if the path doesn't exist or represents a file
     */
    bool isDirectory() const;

    /**
     * Tests whether this path is empty. An empty path does not
     * exist.
     *
     * @return true if the underlying abstract pathname is empty,
     *         false otherwise
     */
    bool isEmpty() const { return m_path.empty(); }

    const char* c_str() const { return m_path.c_str(); }

    /**
     * Replaces the abstract pathname of this object with the
     * specified pathname.
     *
     * @param pathname a pathname string
     */
    void setPath(const std::string& pathname) {
        m_path = getCanonicalPath(pathname);
    }

    /**
     * @return the canonical pathname this path represents
     */
    const std::string& getPath() const { return m_path; }

    /**
     * Returns the parent of this path as Path.
     * @return a new path containing the parent of this path
     */
    Path getParent() const;

    /**
     * Returns ancestor path where "0" is the immediate parent.
     * @return a new path containing the ancestor of this path
     */
    Path getAncestor(int n) const;

    /**
     * Returns the name of the file or directory represented by
     * this abstract pathname.
     *
     * @return the name of the file or directory represented by
     *         this abstract pathname, or an empty string if
     *         this path is empty
     */
    std::string getName() const;

    /**
     * Returns the name of the file or directory represented by
     * this abstract pathname without its extension.
     *
     * @return the name of the file or directory represented by
     *         this abstract pathname, or an empty string if
     *         this path is empty
     */
    std::string getNameWithoutExtension() const;

    /**
     * Returns the file extension (after the ".") if one is present.
     * Returns the empty string if no filename is present or if the
     * path is a directory.
     *
     * @return the file extension (if one is present and
     *         this is not a directory), else the empty string.
     */
    std::string getExtension() const;

    /**
     * Returns the absolute representation of this path.
     * If this path's pathname starts with a leading '/',
     * the returned path is equal to this path. Otherwise,
     * this path's pathname is concatenated with the current
     * working directory and the result is returned.
     *
     * @return a new path containing the absolute representation
     *         of this path
     */
    Path getAbsolutePath() const;

    /**
     * @return true if this abstract pathname is not empty
     *         and starts with a leading '/', false otherwise
     */
    bool isAbsolute() const;

    /**
     * Splits this object's abstract pathname in a vector of file
     * and directory name. If the underlying abstract pathname
     * starts with a '/', the returned vector's first element
     * will be the string "/".
     *
     * @return a vector of strings, empty if this path is empty
     */
    std::vector<std::string> split() const;

    /**
     * Concatenates the specified path with this path in a new
     * path object.
     *
     * @note if the pathname to concatenate with starts with
     * a leading '/' then that pathname is returned without
     * being concatenated to this object's pathname.
     *
     * @param path the path to concatenate with
     *
     * @return the concatenation of the two paths
     */
    Path concat(const Path& path) const;

    /**
     * Concatenates the specified path with this path and
     * stores the result in this path.
     *
     * @note if the pathname to concatenate with starts with
     * a leading '/' then that pathname replaces this object's
     * pathname.
     *
     * @param path the path to concatenate with
     */
    void concatToSelf(const Path& path);

    operator std::string const&() const { return m_path; }

    Path operator+(const Path& rhs) const { return concat(rhs); }
    Path& operator+=(const Path& rhs) {
        concatToSelf(rhs);
        return *this;
    }

    bool operator==(const Path& rhs) const { return m_path == rhs.m_path; }
    bool operator!=(const Path& rhs) const { return m_path != rhs.m_path; }

    bool operator<(const Path& rhs) const { return m_path < rhs.m_path; }
    bool operator>(const Path& rhs) const { return m_path > rhs.m_path; }

    friend std::ostream& operator<<(std::ostream& os, const Path& path);

    /**
     * Returns a canonical copy of the specified pathname by removing
     * unnecessary path segments such as ".", ".." and "/".
     *
     * @param pathname a pathname string
     *
     * @return the canonical representation of the specified pathname
     */
    static std::string getCanonicalPath(const std::string& pathname);

    /**
     * This method is equivalent to calling root.concat(leaf).
     */
    static Path concat(const std::string& root, const std::string& leaf);

    /**
     * @return a path representing the current working directory
     */
    static Path getCurrentDirectory();

    /**
     * @return a path representing the current executable
     */
    static Path getCurrentExecutable();

    /**
     * @return a path representing a directory where temporary files can be stored
     */
    static Path getTemporaryDirectory();

    /**
     * Creates a directory denoted by the given path.
     * This is not recursive and doesn't create intermediate directories.
     *
     * @return True if directory was successfully created.
     *         When false, errno should have details on actual error.
     */
    bool mkdir() const;

    /**
     * Creates a directory denoted by the given path.
     * This is recursive and parent directories will be created if they do not
     * exist.
     *
     * @return True if directory was successfully created or already exists.
     *         When false, errno should have details on actual error.
     */
    bool mkdirRecursive() const;

    /**
     * Deletes this file.
     *
     * @return True if file was successfully deleted.
     *         When false, errno should have details on actual error.
     */
    bool unlinkFile();

    /**
    * Lists the contents of this directory, skipping hidden files.
    *
    * @return A vector of paths of the contents of the directory.  If the path points to a file,
    *         nonexistent directory, or empty directory, an empty vector is returned.
    */
    std::vector<Path> listContents() const;

private:
    std::string m_path;
};

} // namespace utils

#endif // UTILS_PATH_H_
