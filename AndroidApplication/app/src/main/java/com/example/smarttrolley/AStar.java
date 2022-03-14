package com.example.smarttrolley;
import java.util.Comparator;
import java.util.Scanner;
import java.util.Vector;

public class AStar {

    public final static int NORTH = 1;
    public  final static int SOUTH = 2;
    public final static int EAST = 3;
    public final static int WEST = 4;
    private int position = WEST;

    class Node{
        int x, y;
        double gCost, hCost, fCost;
        Node parent;
        char ch;

        boolean sameCoordinate(Node n) {
            return this.x == n.x && this.y == n.y;
        }

        public Node(int x, int y, double gCost, double hCost, char ch, Node parent) {
            this.x = x;
            this.y = y;
            this.gCost = gCost;
            this.hCost = hCost;
            this.fCost = gCost + hCost;
            this.ch = ch;
            this.parent = parent;
        }

        public Node(int x, int y) {
            this.x = x;
            this.y = y;
        }
    }

    final int INF = (int)1e6;
    int row, col, srcX, srcY, dstX, dstY;
    Node map[][];

    Vector<Node> open = new Vector<>();
    Scanner scan = new Scanner(System.in);

    double getHeuristic(int x1, int y1, int x2, int y2) {
        return Math.sqrt(Math.pow(x1 - x2, 2) + Math.pow(y1 - y2, 2));
    }

    void getNeighbor(int x, int y, Node parent, Vector<Node> vec) {
        if(x < 0 || y < 0 || x >= col || y >= row || map[y][x].ch == '#' || map[y][x].ch == 'v') return;
        vec.add(map[y][x]);
    }

    Vector<Node> getNeighbors(Node curr) {
        Vector<Node> ret = new Vector<>();
        getNeighbor(curr.x - 1, curr.y, curr, ret);
        getNeighbor(curr.x + 1, curr.y, curr, ret);
        getNeighbor(curr.x, curr.y - 1, curr, ret);
        getNeighbor(curr.x, curr.y + 1, curr, ret);

        return ret;
    }

    Node[][] generateMap(int row, int col){
        Node[][] map = new Node[row][col];
        for(int i = 0; i < row; i++) {
            for(int j = 0; j < col; j++) {
                map[i][j] = new Node(j, i, INF, getHeuristic(j, i, dstX, dstY), '.', null);
            }
        }
        return map;
    }

    void printMap() {
        System.out.println("");
        for(int i = 0; i < row; i++) {
            for(int j = 0; j < col; j++) {
                System.out.print(map[i][j].ch + " ");
            }
            System.out.println("");
        }
    }

    int getPosition(Node a, Node b) {
        if(b == null) return position;
        int deltaX = b.x - a.x;
        int deltaY = b.y - a.y;

        if(deltaX != 0) return deltaX > 0 ? EAST : WEST;
        if(deltaY != 0) return deltaY > 0 ? SOUTH : NORTH;
        return position;
    }

    String getRotation(Node a, Node b) {
        int nextPosition = getPosition(a, b);
        if(position == nextPosition) return "1";

        String ret = "";

        if((position == WEST && nextPosition == EAST) || (position == EAST && nextPosition == WEST)
                || (position == NORTH && nextPosition == SOUTH) || (position == SOUTH && nextPosition == NORTH))
            ret = "441";
        else if((position == EAST && nextPosition == SOUTH) || (position == NORTH && nextPosition == EAST)
                || (position == WEST && nextPosition == NORTH) || (position == SOUTH && nextPosition == WEST))
            ret = "31";
        else ret = "41";

        position = nextPosition;
        return ret;
    }

    public String backTrack() {
        Node curr = map[dstY][dstX];
        Vector<Node> path = new Vector<>();

        while(curr != null) {
            curr.ch = 'X';
            path.add(0, curr);
            curr = curr.parent;
        }
        printMap();

        StringBuilder p = new StringBuilder();
        while(path.size() > 1) {
            curr = path.remove(0);
            p.append(getRotation(curr, path.get(0)));
        }

        return p.toString();
    }

    public AStar(int srcX, int srcY, int dstX, int dstY, int position) {
        this.position = position;
        this.row = Math.abs(srcY - dstY) + 1;
        this.col = Math.abs(srcX - dstX) + 1;
        System.out.println(row);
        int min = Math.min(srcX, dstX);
        this.srcX = srcX - min;
        this.dstX = dstX - min;
        min = Math.min(srcY, dstY);
        this.dstY = dstY - min;
        this.srcY = srcY - min;

        this.map = generateMap(row, col);

        Node start = map[this.srcY][this.srcX];
        start.gCost = 0;
        start.fCost = start.gCost + start.hCost;

        Node end = map[this.dstY][this.dstX];

        Comparator<Node> comparator = new Comparator<Node>() {
            @Override
            public int compare(Node o1, Node o2) {
                return Double.compare(o1.fCost, o2.fCost);
            }
        };

        Node curr;
        open.add(start);

        while(!open.isEmpty()) {
            open.sort(comparator);

            curr = open.remove(0);
            curr.ch = 'v';
            if(curr.sameCoordinate(end)) break;

            Vector<Node> neighbors = getNeighbors(curr);
            for (Node n : neighbors) {
                if(n.ch == 'v') continue;
                double cost = curr.gCost + 1;

                if(n.ch == 'e' && cost < n.gCost) n.ch = '.';

                if(n.ch != '.') continue;
                n.ch = 'e';
                n.gCost = cost;
                n.fCost = cost + n.hCost;
                n.parent = curr;
                open.add(n);
            }

        }
    }

}
