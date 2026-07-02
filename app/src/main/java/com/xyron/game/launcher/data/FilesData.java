package com.xyron.game.launcher.data;

public class FilesData {
    private final String name;
    private final long size;
    private final String path;
    private final String url;

    public FilesData(String name, long size, String path, String url) {
        this.name = name;
        this.size = size;
        this.path = path;
        this.url = url;
    }

    public String getName() { return name; }
    public long getSize() { return size; }
    public String getPath() { return path; }
    public String getUrl() { return url; }
}
